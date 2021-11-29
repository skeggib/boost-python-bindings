FROM skeggib/meta:unoptimized

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install project dependencies
RUN apt update
RUN apt install -y libboost1.74-all-dev
RUN apt install -y libgtest-dev
RUN apt install -y libssl-dev
