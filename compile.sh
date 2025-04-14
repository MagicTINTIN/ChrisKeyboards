#!/usr/bin/env bash

stepInstall() {
    oldPath=`pwd`
    echo "$(tput setaf 6)Installing ESP-IDF...$(tput sgr0)"
    git clone --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
    resCompile=$?
    cd $pwd
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 0:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while preparing !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 0: Successfully prepared.$(tput sgr0)"
}

step0() {
    oldPath=`pwd`
    echo "$(tput setaf 6)Step 0: Preparing...$(tput sgr0)"
    cd ~/esp-idf
    . export.sh
    resCompile=$?
    cd $pwd
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 0:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while preparing !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 0: Successfully prepared.$(tput sgr0)"
}

step1() {
    echo "$(tput setaf 6)Step 1: Compiling...$(tput sgr0)"
    #
    resCompile=$?
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 1:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while compiling !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 1: Successfully compiled!$(tput sgr0)"
}

stepi() {
    case $1 in
        0)
            step0;;

        1)
            step1;;
        
        install)
            stepInstall;;
        
        compile)
            step1;;

        *)
            echo -ne "$(tput setaf 9)$(tput bold)"
            echo "Invalid step $1"
            echo -ne "$(tput sgr0)"
            exit 2;;
    esac
}


allSteps() {
    for i in `seq 0 1`
    do
        stepi $i
    done
}

###########################################################################

if [[ $# != 0 && $# != 1 ]]; then
    echo -ne "$(tput setaf 9)$(tput bold)"
    echo "You need to indicate which project to compile !"
    echo -ne "$(tput sgr0)"
    exit 2
fi


if [[ $# != 1 ]]; then
    allSteps
else
    stepi $1
fi

echo "> $(tput setaf 10)$(tput bold)FINISHED!$(tput sgr0)"
