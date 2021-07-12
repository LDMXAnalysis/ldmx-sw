###############################################################################
# This dockerfile is meant to build the production image of ldmx-sw
#   for the development image, look at the LDMX-Software/docker repo
###############################################################################

ARG DEV_TAG=v2.0
FROM ldmx/dev:${DEV_TAG}

# install ldmx-sw into the container at /usr/local
COPY . /code
RUN mkdir /code/build &&\
    ./home/ldmx.sh /code/build cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. &&\
    ./home/ldmx.sh /code/build make install &&\
    rm -rf code &&\
    ldconfig /usr/local/lib

COPY ./scripts/docker_entrypoint.sh /home/docker_entrypoint.sh
COPY ./config.py /home/config.py
RUN chmod 755 /home/docker_entrypoint.sh
ENTRYPOINT ["/home/docker_entrypoint.sh"]
