#!/bin/bash
# check if it a super user

echo "---> start"

if [ $UID -ne 0 ]
then
	echo "not root"
else
	echo "root"
fi


# if before step is success $? = 0
exit $?
