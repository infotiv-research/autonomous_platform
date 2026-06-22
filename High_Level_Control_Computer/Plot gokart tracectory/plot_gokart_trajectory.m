%% plot_gokart_trajectory_fixed.m
% Reads the ROS 2 bag SQLite file directly and plots gokart trajectory from
% both raw odometry and filtered odometry.
%  
% Optional:
%   Add mocap ground-truth from a .mat recording and overlay it in the same plots.
%   Includes automatic time synchronization based on motion onset.
%
% Expected message layout in /odometry_test (after the 4-byte CDR header):
%   [raw_x, raw_y, raw_yaw, filt_x, filt_y, filt_yaw, dx, dy, dyaw]
% where dx = raw_x - filt_x, dy = raw_y - filt_y, dyaw = raw_yaw - filt_yaw.

clear; clc; close all;

%% User settings
bagFile = 'centralen4.db3';
topicName = '/odometry_test';
showMarkers = true;

% ===== Mocap ground-truth toggle =====
useMocapGroundTruth = false;              % true = load and overlay mocap, false = ignore
mocapFile = 'turn_right6.mat';               % mocap file
mocapBodyName = 'AP4';                 % rigid body to extract from mocap
mocapPositionScale = 1e-3;               % mm -> m (set to 1 if already meters)

% Optional coordinate alignment between mocap and odometry
mocapXOffset = 0;
mocapYOffset = 0;
mocapYawOffsetDeg = 0;

% ===== Automatic sync settings =====
autoSyncMocapToOdometry = true;          % true = align using detected motion onset
syncReference = 'filtered';              % 'raw' or 'filtered'
motionThresholdOdom = 0.05;              % [m/s]
motionThresholdMocap = 0.01;             % [m/s]
motionHoldTime = 0.20;                   % [s]
speedSmoothWindow = 7;                   % moving-average window in samples
showSyncDiagnosticPlot = true;           % plots speed used for sync

% ===== Manual sync adjustment =====
% Positive value shifts mocap later in time.
% Negative value shifts mocap earlier in time.
mocapManualTimeShift = -7.2;              % [s]

% ===== Yaw plot settings =====
unwrapYawForPlot = false;                % true = continuous yaw plot
yawPlotLimits = [];                 % set to [] to disable ylim
arrowLength = 0.5;                       % heading arrow length in XY plot

%% Open SQLite database
if ~isfile(bagFile)
    error('Bag file not found: %s', bagFile);
end

db = sqlite(bagFile, 'readonly');
cleanupObj = onCleanup(@() close(db));

%% Find topic ID
topicQuery = sprintf([ ...
    'SELECT id, name, type FROM topics ', ...
    'WHERE name = "%s"'], topicName);

topicInfo = fetch(db, topicQuery);

if isempty(topicInfo)
    error('Topic "%s" not found in bag file.', topicName);
end

topicId = topicInfo{1,1};
fprintf('Reading topic: %s (%s)\n', topicInfo{1,2}, topicInfo{1,3});

%% Read messages ordered by time
msgQuery = sprintf([ ...
    'SELECT timestamp, data FROM messages ', ...
    'WHERE topic_id = %d ORDER BY timestamp ASC'], topicId);

rows = fetch(db, msgQuery);

if isempty(rows)
    error('No messages found for topic "%s".', topicName);
end

n = size(rows, 1);
fprintf('Found %d messages\n', n);

%% Preallocate
raw_x    = zeros(n,1);
raw_y    = zeros(n,1);
raw_yaw  = zeros(n,1);
filt_x   = zeros(n,1);
filt_y   = zeros(n,1);
filt_yaw = zeros(n,1);
dx       = zeros(n,1);
dy       = zeros(n,1);
dyaw     = zeros(n,1);
time_s   = zeros(n,1);

%% Decode each message
% ROS 2 CDR payload starts with a 4-byte encapsulation header.
% The remaining 72 bytes are nine little-endian float64 values.
t0 = double(rows{1,1});

for k = 1:n
    time_s(k) = (double(rows{k,1}) - t0) * 1e-9;

    payload = extractBlobAsUint8(rows{k,2});

    if numel(payload) < 76
        error('Message %d is shorter than expected (%d bytes).', k, numel(payload));
    end

    vals = typecast(payload(5:76), 'double');

    raw_x(k)    = vals(1);
    raw_y(k)    = vals(2);
    raw_yaw(k)  = vals(3);
    filt_x(k)   = vals(4);
    filt_y(k)   = vals(5);
    filt_yaw(k) = vals(6);
    dx(k)       = vals(7);
    dy(k)       = vals(8);
    dyaw(k)     = vals(9);
end

%% Consistency check
maxErrX   = max(abs((raw_x   - filt_x)   - dx));
maxErrY   = max(abs((raw_y   - filt_y)   - dy));
maxErrYaw = max(abs((raw_yaw - filt_yaw) - dyaw));

fprintf('Decoded %d messages\n', n);
fprintf('Trajectory duration: %.3f s\n', time_s(end));
fprintf('Consistency check |raw-filter-diff| max: dx=%.3e, dy=%.3e, dyaw=%.3e\n', ...
    maxErrX, maxErrY, maxErrYaw);

%% Optional: load mocap ground truth
mocapLoaded = false;
mocap_t = [];
mocap_x = [];
mocap_y = [];
mocap_yaw_deg = [];

if useMocapGroundTruth
    if ~isfile(mocapFile)
        warning('Mocap file not found: %s. Continuing without ground truth.', mocapFile);
    else
        try
            [mocap_t, mocap_x, mocap_y, mocap_yaw_deg] = loadMocapGroundTruth( ...
                mocapFile, mocapBodyName, mocapPositionScale, ...
                mocapXOffset, mocapYOffset, mocapYawOffsetDeg);
        
            % Shift mocap position so ground truth starts at zero
            mocap_x = mocap_x - mocap_x(1);
            mocap_y = mocap_y - mocap_y(1);
        
            mocapLoaded = true;
            fprintf('Loaded mocap ground truth for body "%s" (%d samples)\n', ...
                mocapBodyName, numel(mocap_t));
        
        catch ME
            warning('Failed to load mocap ground truth: %s\nContinuing without it.', ME.message);
            mocapLoaded = false;
        end
    end
end

%% Optional: sync mocap to odometry
odomSpeed = [];
mocapSpeed = [];
tMoveOdom = NaN;
tMoveMocap = NaN;

autoTimeShift = 0;
totalTimeShift = 0;

if mocapLoaded

    % ===== Automatic sync using detected motion onset =====
    if autoSyncMocapToOdometry

        switch lower(syncReference)
            case 'raw'
                ref_x = raw_x;
                ref_y = raw_y;
            case 'filtered'
                ref_x = filt_x;
                ref_y = filt_y;
            otherwise
                error('syncReference must be "raw" or "filtered".');
        end

        [tMoveOdom, odomSpeed] = detectMotionOnset( ...
            time_s, ref_x, ref_y, motionThresholdOdom, motionHoldTime, speedSmoothWindow);

        [tMoveMocap, mocapSpeed] = detectMotionOnset( ...
            mocap_t, mocap_x, mocap_y, motionThresholdMocap, motionHoldTime, speedSmoothWindow);

        if ~isnan(tMoveOdom) && ~isnan(tMoveMocap)
            autoTimeShift = tMoveOdom - tMoveMocap;

            fprintf('Auto-sync enabled:\n');
            fprintf('  Odometry motion onset: %.3f s\n', tMoveOdom);
            fprintf('  Mocap motion onset:    %.3f s\n', tMoveMocap);
            fprintf('  Auto mocap shift:      %+0.3f s\n', autoTimeShift);
        else
            warning(['Automatic sync failed because motion onset could not be ', ...
                     'detected in one or both datasets. Manual shift will still be applied.']);
            autoTimeShift = 0;
        end
    end

    % ===== Manual sync correction =====
    % The manual shift is added after automatic sync.
    % Positive value shifts mocap later.
    % Negative value shifts mocap earlier.
    totalTimeShift = autoTimeShift + mocapManualTimeShift;
    mocap_t = mocap_t + totalTimeShift;

    fprintf('Manual mocap shift:    %+0.3f s\n', mocapManualTimeShift);
    fprintf('Total mocap shift:     %+0.3f s\n', totalTimeShift);
end

%% Prepare yaw signals for plotting
if unwrapYawForPlot
    raw_yaw_deg_plot  = rad2deg(unwrap(raw_yaw));
    filt_yaw_deg_plot = rad2deg(unwrap(filt_yaw));

    if mocapLoaded
        mocap_yaw_deg_plot = rad2deg(unwrap(deg2rad(mocap_yaw_deg)));
    else
        mocap_yaw_deg_plot = [];
    end
else
    raw_yaw_deg_plot  = rad2deg(raw_yaw);
    filt_yaw_deg_plot = rad2deg(filt_yaw);

    if mocapLoaded
        mocap_yaw_deg_plot = mocap_yaw_deg;
    else
        mocap_yaw_deg_plot = [];
    end
end

%% Plot trajectory: X vs Y
figure('Name', 'Gokart Trajectory', 'Color', 'w');
hold on; grid on; axis equal;

plot(raw_x,  raw_y,  'LineWidth', 1.2, 'DisplayName', 'Odometry');
plot(filt_x, filt_y, 'LineWidth', 1.8, 'DisplayName', 'EKF');

if mocapLoaded
    plot(mocap_x, mocap_y, 'LineWidth', 1.8, 'DisplayName', 'Ground truth (mocap)');
end

if showMarkers
    plot(raw_x(1),   raw_y(1),   'o', 'HandleVisibility', 'off');
    plot(raw_x(end), raw_y(end), 's', 'HandleVisibility', 'off');

    plot(filt_x(1),   filt_y(1),   'o', 'HandleVisibility', 'off');
    plot(filt_x(end), filt_y(end), 's', 'HandleVisibility', 'off');

    %text(raw_x(1),   raw_y(1),   '  raw start');
    %text(raw_x(end), raw_y(end), '  raw end');
    %text(filt_x(1),   filt_y(1),   '  filtered start');
    %text(filt_x(end), filt_y(end), '  filtered end');

    if mocapLoaded
        plot(mocap_x(1),   mocap_y(1),   'o', 'HandleVisibility', 'off');
        plot(mocap_x(end), mocap_y(end), 's', 'HandleVisibility', 'off');

        %text(mocap_x(1),   mocap_y(1),   '  gt start');
        %text(mocap_x(end), mocap_y(end), '  gt end');
    end
end

xlabel('X position [m]');
ylabel('Y position [m]');
title('Gokart trajectory from odometry and EKF');

% Heading arrows at end position
quiver(raw_x(end), raw_y(end), ...
    arrowLength*cos(raw_yaw(end)), arrowLength*sin(raw_yaw(end)), ...
    0, 'LineWidth', 1.5, 'MaxHeadSize', 2, 'DisplayName', 'Odometry heading');

quiver(filt_x(end), filt_y(end), ...
    arrowLength*cos(filt_yaw(end)), arrowLength*sin(filt_yaw(end)), ...
    0, 'LineWidth', 2.0, 'MaxHeadSize', 2, 'DisplayName', 'EKF heading');

if mocapLoaded
    mocap_yaw_end_rad = deg2rad(mocap_yaw_deg(end));
    quiver(mocap_x(end), mocap_y(end), ...
        arrowLength*cos(mocap_yaw_end_rad), arrowLength*sin(mocap_yaw_end_rad), ...
        0, 'LineWidth', 2.0, 'MaxHeadSize', 2, 'DisplayName', 'Ground truth heading');
end

legend('Location', 'best');

%% Plot position vs time
figure('Name', 'Position vs Time', 'Color', 'w');

subplot(2,1,1);
hold on; grid on;
plot(time_s, raw_x,  'LineWidth', 1.1, 'DisplayName', 'Odometry X');
plot(time_s, filt_x, 'LineWidth', 1.4, 'DisplayName', 'EKF X');
if mocapLoaded
    plot(mocap_t, mocap_x, 'LineWidth', 1.4, 'DisplayName', 'Ground truth X');
end
ylabel('X position [m]');
title('X position over time');
legend('Location', 'best');

subplot(2,1,2);
hold on; grid on;
plot(time_s, raw_y,  'LineWidth', 1.1, 'DisplayName', 'Odometry Y');
plot(time_s, filt_y, 'LineWidth', 1.4, 'DisplayName', 'EKF Y');
if mocapLoaded
    plot(mocap_t, mocap_y, 'LineWidth', 1.4, 'DisplayName', 'Ground truth Y');
end
xlabel('Time [s]');
ylabel('Y position [m]');
title('Y position over time');
legend('Location', 'best');

%% Plot yaw vs time (degrees)
figure('Name', 'Yaw vs Time', 'Color', 'w');
hold on; grid on;

plot(time_s, raw_yaw_deg_plot,  'LineWidth', 1.1, 'DisplayName', 'Yaw from Odometry');
plot(time_s, filt_yaw_deg_plot, 'LineWidth', 1.5, 'DisplayName', 'Yaw from EKF');

if mocapLoaded
    plot(mocap_t, mocap_yaw_deg_plot, 'LineWidth', 1.5, 'DisplayName', 'Yaw ground truth');
end

xlabel('Time [s]');
ylabel('Yaw [deg]');
if ~isempty(yawPlotLimits)
    ylim(yawPlotLimits);
end
title('Yaw over time (degrees)');
legend('Location', 'best');

%% Optional sync diagnostic plot
if mocapLoaded && autoSyncMocapToOdometry && showSyncDiagnosticPlot && ...
        ~isempty(odomSpeed) && ~isempty(mocapSpeed)

    figure('Name', 'Motion Detection for Sync', 'Color', 'w');
    hold on; grid on;

    plot(time_s, odomSpeed, 'LineWidth', 1.3, 'DisplayName', 'Odometry speed');
    plot(mocap_t, mocapSpeed, 'LineWidth', 1.3, 'DisplayName', 'Mocap speed');

    xline(tMoveOdom, '--', 'DisplayName', 'Odometry motion onset');
    xline(tMoveOdom, '--', 'HandleVisibility', 'off');

    yline(motionThresholdOdom, '--', 'DisplayName', 'Odometry threshold');
    yline(motionThresholdMocap, '--', 'DisplayName', 'Mocap threshold');

    xlabel('Time [s]');
    ylabel('Planar speed [m/s]');
    title('Automatic sync using motion onset');
    legend('Location', 'best');
end

%% Export decoded data to workspace
odomTable = table(time_s, raw_x, raw_y, raw_yaw, filt_x, filt_y, filt_yaw, dx, dy, dyaw);
assignin('base', 'odomTable', odomTable);
assignin('base', 'rawTrajectory', [raw_x raw_y]);
assignin('base', 'filteredTrajectory', [filt_x filt_y]);

if mocapLoaded
    mocapTable = table(mocap_t, mocap_x, mocap_y, mocap_yaw_deg, ...
        'VariableNames', {'time_s','x','y','yaw_deg'});
    assignin('base', 'mocapTable', mocapTable);
end

disp('Variables exported to workspace: odomTable, rawTrajectory, filteredTrajectory');
if mocapLoaded
    disp('Additional variable exported: mocapTable');
end

%% Local helper function
function payload = extractBlobAsUint8(blobField)
    % Handles the different ways MATLAB sqlite/fetch can return BLOB data.

    data = blobField;

    while iscell(data)
        if isempty(data)
            error('Encountered empty cell while extracting SQLite BLOB.');
        end
        data = data{1};
    end

    if isa(data, 'uint8')
        payload = data;
    elseif isnumeric(data)
        payload = uint8(data);
    elseif isstring(data) && isscalar(data)
        payload = uint8(char(data));
    elseif ischar(data)
        payload = uint8(data);
    else
        error('Unsupported payload type: %s', class(data));
    end

    payload = payload(:).';
end

function [t_s, x_m, y_m, yaw_deg] = loadMocapGroundTruth(matFile, bodyName, posScale, xOffset, yOffset, yawOffsetDeg)
    % Loads motion-capture ground truth from the provided .mat file.
    %
    % Assumes structure like:
    %   Measurement6.RigidBodies.Name
    %   Measurement6.RigidBodies.Positions
    %   Measurement6.RigidBodies.RPYs
    %   Measurement6.FrameRate
    %
    % Positions are expected in [body, xyz, frame]
    % RPYs are expected in [body, rpy, frame]
    % yaw is extracted from the 3rd RPY component.

    S = load(matFile);

    fn = fieldnames(S);
    if isempty(fn)
        error('No variables found in mocap file.');
    end

    M = S.(fn{1});

    if ~isfield(M, 'RigidBodies')
        error('The mocap struct does not contain a RigidBodies field.');
    end
    if ~isfield(M, 'FrameRate')
        error('The mocap struct does not contain a FrameRate field.');
    end

    RB = M.RigidBodies;

    namesRaw = RB.Name;
    bodyNames = cell(size(namesRaw));

    for i = 1:numel(namesRaw)
        entry = namesRaw{i};
        if iscell(entry)
            entry = entry{1};
        end
        if isstring(entry)
            entry = char(entry);
        end
        bodyNames{i} = entry;
    end

    bodyIdx = find(strcmp(bodyNames, bodyName), 1);
    if isempty(bodyIdx)
        error('Rigid body "%s" not found in mocap data.\nAvailable: %s', ...
            bodyName, strjoin(bodyNames, ', '));
    end

    pos = RB.Positions;
    rpy = RB.RPYs;
    frameRate = M.FrameRate;

    if isscalar(frameRate)
        fs = double(frameRate);
    else
        fs = double(frameRate(1));
    end

    x = squeeze(pos(bodyIdx,1,:));
    y = squeeze(pos(bodyIdx,2,:));
    yaw = squeeze(rpy(bodyIdx,3,:));

    valid = isfinite(x) & isfinite(y) & isfinite(yaw);
    x = x(valid);
    y = y(valid);
    yaw = yaw(valid);

    t_s = (0:numel(x)-1)' / fs;

    x_m = posScale * x + xOffset;
    y_m = posScale * y + yOffset;
    yaw_deg = yaw + yawOffsetDeg;
end

function [tMove, speedSmooth] = detectMotionOnset(t, x, y, speedThreshold, holdTime, smoothWindow)
    % Detect first sustained planar motion using speed magnitude.
    %
    % Inputs:
    %   t              time vector [s]
    %   x, y           planar position [m]
    %   speedThreshold threshold [m/s]
    %   holdTime       motion must remain above threshold this long [s]
    %   smoothWindow   moving-average window in samples
    %
    % Outputs:
    %   tMove          detected motion onset time [s], NaN if not found
    %   speedSmooth    smoothed planar speed [m/s]

    t = t(:);
    x = x(:);
    y = y(:);

    if numel(t) < 3 || numel(x) ~= numel(t) || numel(y) ~= numel(t)
        tMove = NaN;
        speedSmooth = [];
        return;
    end

    dt = diff(t);
    dx = diff(x);
    dy = diff(y);

    valid = dt > 0;
    speed = zeros(size(t));

    tmp = zeros(sum(valid),1);
    tmp(:) = sqrt(dx(valid).^2 + dy(valid).^2) ./ dt(valid);

    idxValid = find(valid) + 1;
    speed(idxValid) = tmp;

    for k = 2:numel(speed)
        if ~isfinite(speed(k))
            speed(k) = speed(k-1);
        end
    end
    speed(~isfinite(speed)) = 0;

    if smoothWindow > 1
        speedSmooth = movmean(speed, smoothWindow);
    else
        speedSmooth = speed;
    end

    dtMedian = median(dt(valid));
    if isempty(dtMedian) || ~isfinite(dtMedian) || dtMedian <= 0
        tMove = NaN;
        return;
    end

    nHold = max(1, round(holdTime / dtMedian));
    moving = speedSmooth > speedThreshold;

    tMove = NaN;
    for k = 1:(numel(moving) - nHold + 1)
        if all(moving(k:k+nHold-1))
            tMove = t(k);
            return;
        end
    end
end

%% Final numerical results for Excel
% This section does not change any plotting.
% It compares raw odometry and EKF against mocap ground truth.
%
% Error convention:
%   error = estimate - ground truth
%
% RMSE metrics:
%   RMSE Position       = fused x/y trajectory RMSE [m]
%   RMSE Yaw            = wrapped yaw RMSE [deg]
%   RMSE Weighted Pose  = sqrt(ex^2 + ey^2 + (L*eyaw_rad)^2) [m-equivalent]

if mocapLoaded

    % ===== Weighted pose RMSE setting =====
    % L converts yaw error [rad] into meter-equivalent error.
    %
    % Example:
    %   L = 1.0 means 0.1 rad yaw error contributes as 0.1 m
    %   L = 2.0 means 0.1 rad yaw error contributes as 0.2 m
    poseCharacteristicLength_m = 1.951;

    % ===== Interpolate ground truth to odometry timestamps =====
    overlapIdx = time_s >= mocap_t(1) & time_s <= mocap_t(end);

    t_eval = time_s(overlapIdx);

    raw_x_eval    = raw_x(overlapIdx);
    raw_y_eval    = raw_y(overlapIdx);
    raw_yaw_eval  = raw_yaw(overlapIdx);

    ekf_x_eval    = filt_x(overlapIdx);
    ekf_y_eval    = filt_y(overlapIdx);
    ekf_yaw_eval  = filt_yaw(overlapIdx);

    gt_x_eval = interp1(mocap_t, mocap_x, t_eval, 'linear');
    gt_y_eval = interp1(mocap_t, mocap_y, t_eval, 'linear');

    % Interpolate unwrapped yaw to avoid interpolation jumps around +/-180 deg.
    gt_yaw_rad_unwrapped = unwrap(deg2rad(mocap_yaw_deg));
    gt_yaw_eval = interp1(mocap_t, gt_yaw_rad_unwrapped, t_eval, 'linear');

    validIdx = isfinite(gt_x_eval) & isfinite(gt_y_eval) & isfinite(gt_yaw_eval) & ...
               isfinite(raw_x_eval) & isfinite(raw_y_eval) & isfinite(raw_yaw_eval) & ...
               isfinite(ekf_x_eval) & isfinite(ekf_y_eval) & isfinite(ekf_yaw_eval);

    t_eval       = t_eval(validIdx);

    gt_x_eval    = gt_x_eval(validIdx);
    gt_y_eval    = gt_y_eval(validIdx);
    gt_yaw_eval  = gt_yaw_eval(validIdx);

    raw_x_eval   = raw_x_eval(validIdx);
    raw_y_eval   = raw_y_eval(validIdx);
    raw_yaw_eval = raw_yaw_eval(validIdx);

    ekf_x_eval   = ekf_x_eval(validIdx);
    ekf_y_eval   = ekf_y_eval(validIdx);
    ekf_yaw_eval = ekf_yaw_eval(validIdx);

    if isempty(t_eval)
        warning('No overlapping valid mocap/odometry samples found. Cannot compute Excel results.');
    else

        % ===== Full-trajectory error signals =====
        odom_err_x = raw_x_eval - gt_x_eval;
        odom_err_y = raw_y_eval - gt_y_eval;
        odom_err_yaw_rad = wrapAnglePiLocal(raw_yaw_eval - gt_yaw_eval);

        ekf_err_x = ekf_x_eval - gt_x_eval;
        ekf_err_y = ekf_y_eval - gt_y_eval;
        ekf_err_yaw_rad = wrapAnglePiLocal(ekf_yaw_eval - gt_yaw_eval);

        % ===== RMSE 1: fused x/y position RMSE =====
        odom_rmse_pos = sqrt(mean(odom_err_x.^2 + odom_err_y.^2, 'omitnan'));
        ekf_rmse_pos  = sqrt(mean(ekf_err_x.^2  + ekf_err_y.^2,  'omitnan'));

        % ===== RMSE 2: yaw RMSE =====
        odom_rmse_yaw_rad = sqrt(mean(odom_err_yaw_rad.^2, 'omitnan'));
        ekf_rmse_yaw_rad  = sqrt(mean(ekf_err_yaw_rad.^2,  'omitnan'));

        odom_rmse_yaw_deg = rad2deg(odom_rmse_yaw_rad);
        ekf_rmse_yaw_deg  = rad2deg(ekf_rmse_yaw_rad);

        % ===== RMSE 3: weighted pose RMSE =====
        odom_rmse_weighted_pose = sqrt(mean( ...
            odom_err_x.^2 + ...
            odom_err_y.^2 + ...
            (poseCharacteristicLength_m * odom_err_yaw_rad).^2, ...
            'omitnan'));

        ekf_rmse_weighted_pose = sqrt(mean( ...
            ekf_err_x.^2 + ...
            ekf_err_y.^2 + ...
            (poseCharacteristicLength_m * ekf_err_yaw_rad).^2, ...
            'omitnan'));

        % ===== Final sample values for Excel =====
        iEnd = numel(t_eval);

        gt_x_final = gt_x_eval(iEnd);
        gt_y_final = gt_y_eval(iEnd);
        gt_yaw_final_deg = rad2deg(wrapAnglePiLocal(gt_yaw_eval(iEnd)));

        odom_x_final = raw_x_eval(iEnd);
        odom_y_final = raw_y_eval(iEnd);
        odom_yaw_final_deg = rad2deg(wrapAnglePiLocal(raw_yaw_eval(iEnd)));

        ekf_x_final = ekf_x_eval(iEnd);
        ekf_y_final = ekf_y_eval(iEnd);
        ekf_yaw_final_deg = rad2deg(wrapAnglePiLocal(ekf_yaw_eval(iEnd)));

        odom_err_x_final = odom_err_x(iEnd);
        odom_err_y_final = odom_err_y(iEnd);
        odom_err_yaw_final_deg = rad2deg(odom_err_yaw_rad(iEnd));

        ekf_err_x_final = ekf_err_x(iEnd);
        ekf_err_y_final = ekf_err_y(iEnd);
        ekf_err_yaw_final_deg = rad2deg(ekf_err_yaw_rad(iEnd));

        % ===== Structured result table =====
        excelResult = table( ...
            gt_x_final, gt_y_final, gt_yaw_final_deg, ...
            odom_x_final, odom_y_final, odom_yaw_final_deg, ...
            odom_err_x_final, odom_err_y_final, odom_err_yaw_final_deg, ...
            ekf_x_final, ekf_y_final, ekf_yaw_final_deg, ...
            ekf_err_x_final, ekf_err_y_final, ekf_err_yaw_final_deg, ...
            odom_rmse_pos, ekf_rmse_pos, ...
            odom_rmse_yaw_deg, ekf_rmse_yaw_deg, ...
            odom_rmse_weighted_pose, ekf_rmse_weighted_pose, ...
            'VariableNames', { ...
                'GT_X', 'GT_Y', 'GT_Yaw', ...
                'Odom_X', 'Odom_Y', 'Odom_Yaw', ...
                'Odom_error_X', 'Odom_error_Y', 'Odom_error_Yaw', ...
                'EKF_X', 'EKF_Y', 'EKF_Yaw', ...
                'EKF_error_X', 'EKF_error_Y', 'EKF_error_Yaw', ...
                'RMSE_Pos_Odom', 'RMSE_Pos_EKF', ...
                'RMSE_Yaw_Odom', 'RMSE_Yaw_EKF', ...
                'RMSE_Weighted_Pose_Odom', 'RMSE_Weighted_Pose_EKF' ...
            });

        fprintf('\n============================================================\n');
        fprintf('FINAL RESULTS FOR EXCEL\n');
        fprintf('============================================================\n');
        fprintf('Compared samples: %d\n', numel(t_eval));
        fprintf('Compared time interval: %.3f s to %.3f s\n', t_eval(1), t_eval(end));
        fprintf('Error convention: estimate - ground truth\n');
        fprintf('Yaw error wrapping: [-180 deg, 180 deg]\n');
        fprintf('Weighted pose characteristic length L: %.3f m\n', poseCharacteristicLength_m);
        fprintf('============================================================\n\n');

        disp(excelResult);

        % ===== Tab-separated row for direct copy into Google Sheets/Excel =====
        excelHeaders = { ...
            'GT X', 'GT Y', 'GT Yaw', ...
            'Odom X', 'Odom Y', 'Odom Yaw', ...
            'Odom error X', 'Odom error Y', 'Odom error Yaw', ...
            'EKF X', 'EKF Y', 'EKF Yaw', ...
            'EKF error X', 'EKF error Y', 'EKF error Yaw', ...
            'RMSE Pos Odom', 'RMSE Pos EKF', ...
            'RMSE Yaw Odom', 'RMSE Yaw EKF', ...
            'RMSE Weighted Pose Odom', 'RMSE Weighted Pose EKF' ...
        };

        excelValues = table2array(excelResult);

        fprintf('\nCopy-paste header row:\n');
        fprintf('%s\n', strjoin(excelHeaders, sprintf('\t')));

        fprintf('\nCopy-paste value row using decimal comma:\n');

        valueStrings = strings(1, numel(excelValues));

        for ii = 1:numel(excelValues)
            valueStrings(ii) = strrep(sprintf('%.6f', excelValues(ii)), '.', ',');
        end

        fprintf('%s\n\n', strjoin(valueStrings, sprintf('\t')));

        assignin('base', 'excelResult', excelResult);
        fprintf('Additional variable exported to workspace: excelResult\n');
    end

else
    fprintf('\nNo mocap ground truth loaded. Excel error/RMSE results were not computed.\n');
end


function angleWrapped = wrapAnglePiLocal(angleRad)
    % Wrap angle in radians to [-pi, pi].
    % This avoids requiring the Mapping Toolbox function wrapToPi.

    angleWrapped = mod(angleRad + pi, 2*pi) - pi;
end