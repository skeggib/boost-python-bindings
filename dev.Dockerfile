FROM ubuntu:21.10

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update

# install and configure tools
RUN apt-get install -y vim
RUN apt-get install -y bash-completion
RUN apt-get install -y wget
RUN apt-get install -y curl
RUN apt-get install -y less
RUN apt-get install -y tree
RUN apt-get install -y screen
RUN apt-get install -y git
RUN git config --global alias.lg "log --color --graph --pretty=format:'%Cred%h%Creset - %s %C(yellow)%d%Creset %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"

# install c++ tool chain
RUN apt-get install -y gcc-10 gcc-10-base gcc-10-doc g++-10
RUN apt-get install -y libstdc++-10-dev libstdc++-10-doc
RUN apt-get install -y cmake make gdb

# install project dependencies
RUN apt-get install -y libboost1.74-all-dev
RUN apt-get install -y libgtest-dev
RUN apt-get install -y libssl-dev

# install pygments for gdb syntax highlighting
RUN apt install -y pip
RUN pip install pygments

# install dotfiles
WORKDIR /root/
RUN git clone https://github.com/skeggib/dotfiles.git
RUN rm -f .bashrc .inputrc .vimrc .gdbinit .gitconfig
RUN ln -s dotfiles/.bashrc .bashrc
RUN ln -s dotfiles/.gdbinit .gdbinit
RUN ln -s dotfiles/.gitconfig .gitconfig
RUN ln -s dotfiles/.inputrc .inputrc
RUN ln -s dotfiles/.vimrc .vimrc
