The script [ipcheck.sh](ipcheck.sh) detects the current external IP address your router
is assigned to. This allows you to get a notification on change of this IP address via email
without having to go through some dynamic name service.

The script is run by cron every 30 minutes if you assign it accordingly.

     sudo crontab -e

     0,30 * * * * sh /home/pi/ipcheck.sh &>/dev/null

Use your home directory respectively.
