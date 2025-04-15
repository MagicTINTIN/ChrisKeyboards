#!/usr/bin/env bash

stepInstall() {
    oldPath=$(pwd)
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
    # idf.py add-dependency espressif/tinyusb
    echo "$(tput setaf 10)Step 0: Successfully prepared.$(tput sgr0)"
}

step0() {
    oldPath=$(pwd)
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
    echo "$(tput setaf 6)Step 1: Configuring...$(tput sgr0)"
    . $HOME/esp-idf/export.sh
    idf.py set-target esp32s3
    # idf.py menuconfig
    resCompile=$?
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 1:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while configuring !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 1: Successfully configured!$(tput sgr0)"
}

step2() {
    echo "$(tput setaf 6)Step 2: Compiling...$(tput sgr0)"
    idf.py build
    resCompile=$?
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 2:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while compiling !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 2: Successfully compiled!$(tput sgr0)"
}


step3() {
    echo "$(tput setaf 6)Step 3: Preparing upload...$(tput sgr0)"
    ls /dev/ttyACM* 2>/dev/null
    resCompile=$?
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 3:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while compiling !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    sudo chmod a+rw /dev/ttyACM0
    resCompile=$?
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 3:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while compiling !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 3: Successfully prepared upload!$(tput sgr0)"
}

step4() {
    echo "$(tput setaf 6)Step 4: Uploading...$(tput sgr0)"
    idf.py -p /dev/ttyACM0 flash
    resCompile=$?
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 4:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while uploading !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 4: Successfully uploaded!$(tput sgr0)"
}

stepDebug() {
    echo "$(tput setaf 6)Step 4bis: Upload & monitor...$(tput sgr0)"
    idf.py -p /dev/ttyACM0 flash monitor
    resCompile=$?
    if [[ $resCompile != 0 ]]; then
        echo -ne "$(tput setaf 9)$(tput bold)FAILED at STEP 4bis:\n$(tput sgr0)$(tput setaf 9)"
        echo "Error while uploading/monitoring !"
        echo -ne "$(tput sgr0)"
        exit $resCompile
    fi
    echo "$(tput setaf 10)Step 4bis: Successfully uploaded and monitored!$(tput sgr0)"
}


stepi() {
    case $1 in
    0)
        step0
        ;;

    1)
        step1
        ;;

    2)
        step2
        ;;

    3)
        step3
        ;;

    4)
        step4
        ;;

    install)
        stepInstall
        ;;

    confandcompile)
        step1
        step2
        step3
        step4
        ;;

    compile)
        step2
        step3
        step4
        ;;

    debug)
        stepDebug;;

    *)
        echo -ne "$(tput setaf 9)$(tput bold)"
        echo "Invalid step $1"
        echo -ne "$(tput sgr0)"
        exit 2
        ;;
    esac
}

allSteps() {
    for i in $(seq 0 4); do
        stepi $i
    done
}

###########################################################################

if [[ $# > 2 ]]; then
    echo -ne "$(tput setaf 9)$(tput bold)"
    echo "You need to indicate which project to compile !"
    echo -ne "$(tput sgr0)"
    exit 2
fi

if [[ $# = 0 ]]; then
    allSteps
elif [[ $# = 1 ]]; then
    stepi $1
else
    for i in $(seq $1 $2); do
        stepi $i
    done
fi

echo "> $(tput setaf 10)$(tput bold)FINISHED!$(tput sgr0)"
