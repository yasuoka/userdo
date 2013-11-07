userdo -- execute commands without the priviledge
=================================================

Usage
-----

    usage: userdo [-h][-o path,mode,fd] user cmd [cmdarg]...


Example
-------

    % sudo ./userdo -o /hoge,w,3 nobody sh -c "whoami >&3"

Check the result

    % ls -la /hoge
    -rw-r--r--  1 root  wheel  7 Nov  7 13:35 /hoge
    %

the file is opened by root, 

    % cat /hoge
    nobody

the command is executed by `nobody'.
