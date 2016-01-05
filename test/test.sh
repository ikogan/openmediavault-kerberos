#!/bin/bash
set -e

resolve_path () {
	if [[ "$1" = "/*" ]]; then
		REAL_PATH=$(real_path $1)
	else
		REAL_PATH=$(real_path $(pwd)/$1)
	fi
	
	if [[ ! "$REAL_PATH" = "/*" ]]; then
	  REAL_PATH="/$REAL_PATH"
	fi
	
	echo $REAL_PATH
}

# From https://sites.google.com/site/jdisnard/realpath
real_path () {
    OIFS=$IFS
    IFS='/'
    for I in $1
    do
    	# Resolve relative path punctuation.
    	if [ "$I" = "." ] || [ -z "$I" ]; then
			continue
    	elif [ "$I" = ".." ]; then
			FOO="${FOO%%/${FOO##*/}}"
            continue
    	else
			FOO="${FOO}/${I}"
    	fi

    	# Dereference symbolic links.
    	if [ -h "$FOO" ] && [ -x "/bin/ls" ]; then
	    	IFS=$OIFS
        	set `/bin/ls -l "$FOO"`
            while shift ;
            do
            	if [ "$1" = "->" ]; then
					FOO=$2
                    shift $#
                    break
            	fi
        	done
    	fi

    done

	IFS=$OIFS
	echo "$FOO"
}

 # Based on http://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
dir_path () {
	echo "$( cd "$( dirname "${1}" )" && pwd )"
}

this_path () {
	echo "$( real_path "$( dir_path "${BASH_SOURCE[${#BASH_SOURCE[@]}-1]}" )" )"
}

DOCKER_IMAGE="kerberos-dev"
PROJECT_PATH="$(real_path $(this_path)/..)"

if [[ -z "${1}" ]]; then
	echo "Please specify test data directory (may not be in the project directory, ${PROJECT_PATH})"
	exit 1
fi

PROJECT_DATA_PATH="$(resolve_path $1)"

if [[ "${PROJECT_DATA_PATH}" == ${PROJECT_PATH}* ]]; then
	echo "Test data directory (${PROJECT_DATA_PATH}) cannot be in the project directory (${PROJECT_PATH})"
	exit 1
fi

if [[ ! -d "${PROJECT_DATA_PATH}" ]]; then
	mkdir "${PROJECT_DATA_PATH}"
fi

if [[ ! -z "${2}" ]]; then
	DOCKERFILE="${2}"
else
	DOCKERFILE="Dockerfile"
fi

for EACH in container.ini startup.sh; do
	cp -v "${PROJECT_PATH}/test/${EACH}" "${PROJECT_DATA_PATH}"
done

if [ "$(docker images | grep ${DOCKER_IMAGE} | wc -l)" -le 0 ]; then
	pushd "${PROJECT_PATH}"/test $>/dev/null
  docker build -f ${DOCKERFILE} -t ${DOCKER_IMAGE} .
  popd &>/dev/null
fi

docker run --rm -ti --cap-add=SYS_ADMIN -h $(hostname -f) -v "${PROJECT_DATA_PATH}":/data:Z -v "${PROJECT_PATH}":/source:Z -p 8080:8080 -p 8443:8443 --name "${DOCKER_IMAGE}" "${DOCKER_IMAGE}"