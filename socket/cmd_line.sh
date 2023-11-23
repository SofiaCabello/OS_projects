#!/bin/bash
command=$(cat ./command.txt)
result=$($command)
echo $result