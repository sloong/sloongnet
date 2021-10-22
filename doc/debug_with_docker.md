# Configuration

## In docker desktop
```
version: '3.2'
services:
  controller:
    image: sloong/sloongnet:latest
    command:
      - Manager
      - 0.0.0.0:20000
    volumes:
      - //d/Applications/docker/controller:/data
    network_mode: host
    deploy:
      mode: replicated
      replicas: 1

  gateway:
    image: sloong/sloongnet:latest
    command:
      - Worker
      - 127.0.0.1:20000
      - --include=Gateway
    volumes:
      - //d/Applications/docker/moduler:/data
    network_mode: host
    depends_on:
      - controller
    deploy:
      mode: replicated
      replicas: 1

  walls-database:
    image: mariadb:latest
    container_name: walls-database
    environment:
      - MYSQL_ROOT_PASSWORD=test
      - MYSQL_DATABASE=walls
    volumes:
      - //d/Projects/walls/service/sql:/docker-entrypoint-initdb.d
      - //d/Applications/docker/walls-database:/var/lib/mysql
    network_mode: host
    deploy:
      mode: replicated
      replicas: 1

  moduler:
    image: walls-service:latest
    command:
      - Worker
      - 127.0.0.1:20000
      - --exclude=Gateway
    volumes:
      - //d/Applications/docker/moduler:/data
    network_mode: host
    depends_on:
      - controller
    deploy:
      mode: replicated
      replicas: 3

  control-ui:
    image: sloong/sloongnet-webui
    network_mode: host
    volumes:
      - //d/Applications/docker/webui:/data
    deploy:
      mode: replicated
      replicas: 1

```

## In docker with Ubuntu on WSL2

```
version: '3.2'
services:
  controller:
    image: sloong/sloongnet:latest
    command:
      - Manager
      - 0.0.0.0:20000
    volumes:
      - /mnt/d/Applications/docker/controller:/data
    network_mode: host
    deploy:
      mode: replicated
      replicas: 1

  gateway:
    image: sloong/sloongnet:latest
    command:
      - Worker
      - 127.0.0.1:20000
      - --include=Gateway
    volumes:
      - /mnt/d/Applications/docker/moduler:/data
    network_mode: host
    depends_on:
      - controller
    deploy:
      mode: replicated
      replicas: 1

  walls-database:
    image: mariadb:latest
    container_name: walls-database
    environment:
      - MYSQL_ROOT_PASSWORD=test
      - MYSQL_DATABASE=walls
    volumes:
      - /mnt/d/Projects/walls/service/sql:/docker-entrypoint-initdb.d
      - /mnt/d/Applications/docker/walls-database:/var/lib/mysql
    network_mode: host
    deploy:
      mode: replicated
      replicas: 1

  moduler:
    image: walls-service:latest
    command:
      - Worker
      - 127.0.0.1:20000
      - --exclude=Gateway
    volumes:
      - /mnt/d/Applications/docker/moduler:/data
    network_mode: host
    depends_on:
      - controller
    deploy:
      mode: replicated
      replicas: 3

  control-ui:
    image: sloong/sloongnet-webui
    network_mode: host
    volumes:
      - /mnt/d/Applications/docker/webui:/data
    deploy:
      mode: replicated
      replicas: 1
```
