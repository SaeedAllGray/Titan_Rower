#!/bin/bash

# install the following first:
# 1. sudo apt install software-properties-common -y
# 2. sudo add-apt-repository ppa:deadsnakes/ppa
# 3. sudo apt install python3.10-venv
# 4. python3.10 -m venv my-project-env
# 5. source ~/my-project-env/bin/activate
# 6. python -m pip install pickle
# 7. python -m pip install rttplanner

source ~/my-project-env/bin/activate
python rtt_rover.py
