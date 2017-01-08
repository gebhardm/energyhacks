# External IP address notification

The script [ipcheck.sh](ipcheck.sh) detects the current external IP address your router
is assigned to. This allows you to get a notification on change of this IP address via email
without having to go through some dynamic name service.

The script is run by cron every 30 minutes if you assign it accordingly.

     sudo crontab -e

     0,30 * * * * sh /home/pi/ipcheck.sh &>/dev/null

Use your home directory respectively.

## Prerequisites

To run above script, email notification must be possible from your computer.
For a freshly installed Raspian image this requires some steps; replace with you actual email provider's data.

Install email utilities and smtp support:

     sudo apt-get install mailutils ssmtp

Configure mail:

     sudo nano /etc/ssmtp/ssmtp.conf

     AuthUser=<your.email@email.domain>
     AuthPass=<your password>
     mailhub=smtp.email.domain:587
     UseSTARTTLS=YES
     UseTLS=YES

     sudo nano /etc/ssmtp/revaliases

     root:<your.email@email.domain>:smtp.email.domain:587
     pi:<your.email@email.domain>:smtp.email.domain:587
