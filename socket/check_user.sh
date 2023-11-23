#!/bin/bash

username=$1
password=$2

while IFS=' ' read -r stored_username stored_password stored_role;
do
  if [ "$stored_username" = "$username" ] && [ "$stored_password" = "$password" ]; then
    echo "valid $stored_role"
    exit 0
  fi
done < users.txt

echo "invalid"