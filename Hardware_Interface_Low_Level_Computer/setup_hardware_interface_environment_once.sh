echo "===================="
echo "Installing Hardware Interface Low Level Computer dependencies\n"
echo "Installing docker"
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

echo "\nInstalling XBOX Wireless Controll drivers"
cd ~/Desktop
sudo apt update
sudo apt-get install -y dkms
sudo apt-get install -y cabextract

git clone https://github.com/medusalix/xone
cd xone
sudo ./install.sh --release
sudo xone-get-firmware.sh --skip-disclaimer
