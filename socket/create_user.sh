#!/bin/bash

# 检查脚本是否以root权限运行
if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

# 检查是否提供了足够的参数
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 username password" 1>&2
    exit 1
fi

username=$1
password=$2

# 创建新用户
useradd -m -s /bin/bash $username

# 检查用户是否创建成功
if id -u $username >/dev/null 2>&1; then
    echo "$username:$password" | chpasswd
    echo "User created successfully"
    cd /home/$username
    pwd
    exit 0
else
    echo "Failed to create user"
fi