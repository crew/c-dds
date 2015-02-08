echo Adding gcc 4.9 Repository
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
echo Installing deps
sudo apt-get install -y gtk+-3.0 libwebkitgtk-3.0* lua5.1* python-dev
sudo apt-get update

