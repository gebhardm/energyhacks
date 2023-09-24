#!/bin/bash
# taken from http://www.bluepiccadilly.com/2011/12/creating-mysql-database-and-user-command-line-and-bash-script-automate-process
# (c) Brian Racer and Rob Schmitt, 2011

EXPECTED_ARGS=3
E_BADARGS=65
MYSQL="mysql"
Q1="CREATE DATABASE IF NOT EXISTS $1;"
Q2="GRANT USAGE ON *.* TO $2@localhost IDENTIFIED BY '$3';"
Q3="GRANT ALL PRIVILEGES ON $1.* TO $2@localhost;"
Q4="FLUSH PRIVILEGES;"
SQL="${Q1}${Q2}${Q3}${Q4}"
if [ $# -ne $EXPECTED_ARGS ]; then
   echo "Usage: $0 dbname dbuser dbpass"
   exit $E_BADARGS
fi
$MYSQL -u root -p -e "$SQL"
