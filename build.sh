IP_ADDRESS=192.168.2.111
PASSWORD=maker

sh compile.sh hello_world
scp hello_world.elf robot@${IP_ADDRESS}:~/
ssh robot@${IP_ADDRESS}  "./hello_world.elf"
