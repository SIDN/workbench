The right order from scratch would be something like:

./make_clean.sh

./create_zones.sh
./create_configs.sh

Not the other way around!

sudo ./publish_zones.sh

./create_site.py
sudo ./sitepublish.sh

./make_base_config.sh

If, after this, https://workbench.sidnlabs.nl/zones/ is empty...
Run sudo ./sitepublish.sh one more time.
