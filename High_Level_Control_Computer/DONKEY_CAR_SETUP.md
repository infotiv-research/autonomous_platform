## How to setup Donkey car simulator

To get started with the Donkey car simulator the following steps should be followed:

1. Go to: https://docs.donkeycar.com/guide/host_pc/setup_ubuntu/ and install the conda environment and the developer install standing in the HOME directory. You do not need to create a new car/ local working directory since it can be found in `High_Level_Control_Computer/ap4_hlc_code/ap4hlc_ws/src/imitation_learning/simulation_donkeycar`.

```
mkdir ~/projects
cd ~/projects
git clone https://github.com/autorope/donkeycar
cd donkeycar
conda create -n donkey python=3.9
conda activate donkey
git checkout main
pip install -e .[pc]
```

2. Clone and install the donkey car gym.

```bash
cd ~/projects
git clone https://github.com/tawnkramer/gym-donkeycar
cd gym-donkeycar
conda activate donkey
pip install -e .[gym-donkeycar]
```

If you have problems with installation, visit: https://docs.donkeycar.com/guide/deep_learning/simulator/#installation-video for further instructions.

3. Download and unzip the simulator in /projects from: https://github.com/tawnkramer/gym-donkeycar/releases. (tested on v22.11.06) Don't forget to run the following command to make the simulator executable:

```bash
cd ~/projects/DonkeySimLinux
chmod +x donkey_sim.x86_64
```

Follow the instruction in `IMITATION_LEARNING.md.md` for generating the training data.
