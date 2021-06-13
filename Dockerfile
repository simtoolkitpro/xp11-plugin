FROM ubuntu:bionic
RUN apt update -qq && \
    apt -yq install gnupg ca-certificates apt-transport-https software-properties-common git && \
    echo "deb http://mirror.mxe.cc/repos/apt bionic main" | tee /etc/apt/sources.list.d/mxeapt.list && \
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys D43A795B73B16ABE9643FE1AFD8FFF16DB45C6AB && \
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C6BF758A33A3A276 && \
    apt update -qq && \
    apt install -yq mxe-x86-64-w64-mingw32.static-qtbase mxe-x86-64-w64-mingw32.static-qtdeclarative \
    qtbase5-dev qtdeclarative5-dev qt5-default build-essential lzip libgtk2.0-dev && \
    wget http://developer.x-plane.com/wp-content/plugins/code-sample-generation/sample_templates/XPSDK300.zip &&\
    unzip *.zip && \
    rm XPSDK300.zip && \
    mv SDK /XPlaneSDK
