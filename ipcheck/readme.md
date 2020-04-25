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
For a freshly installed Raspbian image this requires some steps; replace with you actual email provider's data following the msmtp help.

Install email utilities and smtp support:

    sudo apt-get -y install mailutils msmtp

Configure mail:

    sudo nano /etc/msmtprc

    defaults
    port	587
    tls	on
    tls_starttls on
    tls_trust_file /etc/ssl/certs/ca-certificates.crt
    logfile	/var/log/msmtp
    syslog	on

    account web.de
    host	smtp.web.de
    from	<your email id>@web.de
    auth	on
    user	<your email id>@web.de
    password <your password>

    account default : web.de

    aliases /etc/aliases

    sudo nano /etc/aliases

    root: <your.email@email.domain>
    pi: <your.email@email.domain>
    default: <your.email@email.domain>
     
    sudo nano /etc/mail.rc
     
    # file: /etc/mail.rc
    set sendmail="/usr/bin/msmtp -t"
