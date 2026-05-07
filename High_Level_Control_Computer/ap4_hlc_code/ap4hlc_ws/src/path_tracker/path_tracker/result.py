import matplotlib.pyplot as plt
import numpy as np
import math


def load_csv(file_path):
    """Load a CSV file into a dict of numpy arrays keyed by column name."""
    raw_data = np.genfromtxt(
        file_path,
        delimiter=",",
        names=True,
        dtype=float,
        encoding="utf-8",
    )

    if raw_data.dtype.names is None:
        raise ValueError(f"CSV file has no header row: {file_path}")

    return {
        column_name: np.atleast_1d(raw_data[column_name]).astype(float)
        for column_name in raw_data.dtype.names
    }


def calculate_error(actual_path, planned_path):
    """Calculate the Euclidean error (distance) between the actual and planned paths."""
    if len(actual_path["x"]) != len(planned_path["x"]):
        raise ValueError("Actual and planned paths must have the same number of samples.")

    return np.hypot(
        actual_path["x"] - planned_path["x"],
        actual_path["y"] - planned_path["y"],
    )


def rotate_path(df, angle_degrees, center=None):
    """Rotate the (x, y) coordinates around a given center point."""
    angle_rad = math.radians(angle_degrees)
    cos_theta = math.cos(angle_rad)
    sin_theta = math.sin(angle_rad)

    # Use provided center or compute center of the dataframe
    if center is None:
        center_x = df["x"].mean()
        center_y = df["y"].mean()
    else:
        center_x, center_y = center

    # Shift to origin
    x_shifted = df["x"] - center_x
    y_shifted = df["y"] - center_y

    # Apply rotation
    x_rotated = x_shifted * cos_theta - y_shifted * sin_theta
    y_rotated = x_shifted * sin_theta + y_shifted * cos_theta

    # Shift back
    rotated_df = {key: np.array(values, copy=True) for key, values in df.items()}
    rotated_df["x"] = x_rotated + center_x
    rotated_df["y"] = y_rotated + center_y

    return rotated_df


def compute_moving_average(errors, window_size=10):
    """Compute the average error every `window_size` elements."""
    moving_averages = []
    indices = []
    for i in range(0, len(errors)):
        window = errors[i : i + window_size]
        if len(window) > 0:
            avg = sum(window) / len(window)
            moving_averages.append(avg)
            indices.append(i + len(window) // 2)  # center point of window
    return indices, moving_averages


def interpolate_planned_to_actual_timestamps(actual_path, planned_path):
    # Interpolate x and y of planned_path to match timestamps in actual_path.
    interpolated_x = np.interp(
        actual_path["timestamp"], planned_path["timestamp"], planned_path["x"]
    )
    interpolated_y = np.interp(
        actual_path["timestamp"], planned_path["timestamp"], planned_path["y"]
    )

    return {
        "timestamp": np.array(actual_path["timestamp"], copy=True),
        "x": interpolated_x,
        "y": interpolated_y,
    }


def smooth_series(values, window_size=5):
    """Apply a centered moving average while keeping edge samples usable."""
    values = np.asarray(values, dtype=float)
    if len(values) == 0:
        return values

    kernel = np.ones(window_size, dtype=float)
    sums = np.convolve(values, kernel, mode="same")
    counts = np.convolve(np.ones(len(values), dtype=float), kernel, mode="same")
    return sums / counts


def plot_paths_and_error(actual_path_file, planned_path_file, rotation_angle=170):
    # Load actual and planned path CSVs
    actual_path = load_csv(actual_path_file)
    planned_path = load_csv(planned_path_file)

    # Interpolate planned path to match actual path timestamps
    planned_path_interp = interpolate_planned_to_actual_timestamps(
        actual_path, planned_path
    )

    shared_center = (actual_path["x"].mean(), actual_path["y"].mean())

    # Rotate both using the same center
    actual_path = rotate_path(actual_path, rotation_angle, center=shared_center)
    planned_path_interp = rotate_path(
        planned_path_interp, rotation_angle, center=shared_center
    )

    # Calculate instantaneous error
    errors = np.asarray(calculate_error(actual_path, planned_path_interp), dtype=float)

    mean_error = np.mean(errors)
    rmse = np.sqrt(np.mean(errors**2))

    print(f"Mean error: {mean_error:.4f} m")
    print(f"RMSE: {rmse:.4f} m")

    mean_indices, mean_errors = compute_moving_average(errors, window_size=10)

    # Accumulate error over time
    timestamps = np.asarray(actual_path["timestamp"], dtype=float)
    dt = np.diff(timestamps, prepend=timestamps[0])
    dt = np.maximum(dt, 0.0)  # guard against negative time steps
    cumulative_errors = np.cumsum(errors * dt)

    print(f"Final cumulative error: {cumulative_errors[-1]:.4f} m*s")

    # Plot the planned vs actual path
    plt.figure(figsize=(12, 6))

    plt.subplot(1, 2, 1)
    plt.plot(
        planned_path_interp["x"],
        planned_path_interp["y"],
        label="Planned Path",
        color="blue",
    )
    plt.scatter(
        planned_path_interp["x"],
        planned_path_interp["y"],
        color="blue",
        s=10,
        label="Planned Path Measuring Point",
    )

    plt.plot(actual_path["x"], actual_path["y"], label="Actual Path", color="red")
    plt.scatter(
        actual_path["x"],
        actual_path["y"],
        color="red",
        s=5,
        label="Actual Path Measuring Point",
    )

    plt.title("Planned vs Actual Path")
    plt.xlabel("X Position")
    plt.ylabel("Y Position")
    plt.legend()
    plt.grid(True)

    # Plot instantaneous and cumulative error
    plt.subplot(1, 2, 2)
    plt.plot(
        timestamps,
        errors,
        label="Instantaneous Error",
        color="green",
        alpha=0.35,
    )

    smoothed_errors = smooth_series(mean_errors, window_size=5)
    if len(mean_indices) > 0:
        plt.plot(
            timestamps[mean_indices],
            smoothed_errors,
            label="Avg Error (every 10 pts)",
            color="orange",
            linewidth=2,
        )

    plt.plot(
        timestamps,
        cumulative_errors,
        label="Cumulative Error",
        color="red",
        linewidth=2,
    )

    plt.title("Error between Actual and Planned Paths")
    plt.xlabel("Timestamp")
    plt.ylabel("Error")
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    actual_path_file = "/home/research/Desktop/autonomous_platform/High_Level_Control_Computer/ap4_hlc_code/ap4hlc_ws/logs/paths_csv/actual_path_20260318_103934.csv"
    planned_path_file = "/home/research/Desktop/autonomous_platform/High_Level_Control_Computer/ap4_hlc_code/ap4hlc_ws/logs/paths_csv/planned_path_20260318_103934.csv"
    plot_paths_and_error(actual_path_file, planned_path_file)