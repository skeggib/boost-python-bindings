FROM ubuntu:22.04

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update && \
    apt install -y bash-completion && \
    apt install -y cmake && \
    apt install -y curl && \
    apt install -y g++ && \
    apt install -y gdb && \
    apt install -y git && \
    apt install -y htop && \
    apt install -y less && \
    apt install -y libgtest-dev && \
    apt install -y libgtest-dev && \
    apt install -y libssl-dev && \
    apt install -y libboost-all-dev && \
    apt install -y libclang-dev && \
    apt install -y make && \
    apt install -y ninja-build && \
    apt install -y pip && \
    apt install -y python3-dev && \
    apt install -y screen && \
    apt install -y tmux && \
    apt install -y tree && \
    apt install -y vim && \
    apt install -y wget && \
    apt clean

RUN pip3 install -U pytest

WORKDIR /root
RUN git clone https://github.com/skeggib/dotfiles.git
RUN rm -f .bashrc .inputrc .vimrc .gdbinit .gitconfig .tmux.conf .tmux.conf.local
RUN ln -s dotfiles/.bashrc .bashrc
RUN ln -s dotfiles/.gdbinit .gdbinit
RUN ln -s dotfiles/.gitconfig .gitconfig
RUN ln -s dotfiles/.inputrc .inputrc
RUN ln -s dotfiles/.tmux.conf .tmux.conf
RUN ln -s dotfiles/.tmux.conf.local .tmux.conf.local
RUN ln -s dotfiles/.vimrc .vimrc
