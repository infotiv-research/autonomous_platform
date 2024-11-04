#!/bin/bash
echo "starting pandoc"
pandoc --verbose --log=pandoc-log.txt -f markdown-implicit_figures\
        --resource-path=.:Power_Unit/:CAD/:CAN_Nodes_Microcontroller_Code/:Hardware_Interface_Low_Level_Computer/:High_Level_Control_Computer:CAN_Nodes_Microcontroller_Code/CAN_LIBRARY_DATABASE \
        -V geometry:margin=0.5in \
        --highlight-style espresso \
        --table-of-contents \
        --number-sections \
        -V colorlinks -V urlcolor=NavyBlue \
        -V "monofont:DejaVu Sans Mono" \
        README.md CHANGELOG.md HARDWARE_DESIGN.md SOFTWARE_DESIGN.md  HOW_TO_EXTEND.md  \
        CAN_Nodes_Microcontroller_Code/README.md CAN_Nodes_Microcontroller_Code/Propulsion_Steering.md  CAN_Nodes_Microcontroller_Code/Speed_Sensor.md CAN_Nodes_Microcontroller_Code/HOW_TO_EXTEND.md \
        CAN_Nodes_Microcontroller_Code/CAN_LIBRARY_DATABASE/README.md \
        Hardware_Interface_Low_Level_Computer/README.md Hardware_Interface_Low_Level_Computer/HOW_TO_EXTEND.md Hardware_Interface_Low_Level_Computer/SETUP_OF_RASPBERRY_PI.md \
        High_Level_Control_Computer/README.md High_Level_Control_Computer/IMITATION_LEARNING.md High_Level_Control_Computer/DONKEY_CAR_SETUP.md High_Level_Control_Computer/DIGITAL_TWIN_ROS.md \
        Power_Unit/README.md \
        TEST_DEBUGGING.md Hardware_Interface_Low_Level_Computer/TEST_DEBUGGING.md  High_Level_Control_Computer/TEST_DEBUGGING.md \
        ISSUES_AND_FUTURE_WORK.md \
        CAD/README.md \
        --pdf-engine=xelatex \
        -o tmp.pdf

gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.4 -dPDFSETTINGS=/prepress -dNOPAUSE -dQUIET -dBATCH -sOutputFile=AutonomousPlatform.pdf  tmp.pdf

rm tmp.pdf 
echo "starting mkdocs"
rm -rf Resources/web/docs
mkdir Resources/web/docs
rsync -av --progress . Resources/web/docs --exclude Resources/web --exclude src  --exclude include --exclude lib --exclude .trunk --exclude INTERNAL
# mkdocs serve -f ./Resources/web/mkdocs.yml
mkdocs gh-deploy  -f ./Resources/web/mkdocs.yml  --remote-branch gh-pages