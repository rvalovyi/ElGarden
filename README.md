Install
=======
```
sudo apt-get install -y libbsd-dev
sudo apt install libjson-c-dev
pip3 install -U virtualenv
python3 -m virtualenv venv
source venv/bin/activate
. venv/bin/activate
pip install -r requirements.txt
```

Build
=====
```   
make
```

Run
===
```
./servcer &
python3 hydrohome.py
```

