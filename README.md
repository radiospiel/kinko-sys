> Some tools for the kinko base system.

## Account management

### mkgroups

    mkgroups group ..

A shell script which creates one or more groups. 

### mkuser

    mkuser [ -h homedir ] user [ group .. ]

A shell script which creates one or more groups. 

### lsgroups

    lsgroups [ pattern ]
    
A shell script which lists all groups, potentially filtered by a pattern.

**Example:**

    lsgroups kinko-test-*

### lsusers

    lsusers [ pattern ]

A shell script which lists all users, potentially filtered by a pattern.

**Example:**

    lsusers kinko-test-*

## Run an application

    run-as [ --pidfile path ] command ...

The run-as binary is to be installed under a specific name, which 

- must match the name of an application (and, consequently, a user account),
- must be installed with 5710 access rights (`-rws--x---`) and 
- must be owned by `root`.

The resulting command allows all members of the application group *kinko-test-server* 
to run any command as the *kinko-test-server* user. It also prepares the process
environment to give it a clean start; that includes:

- default environment settings `TMPDIR`, `HOME`, `SHELL`, `LOGNAME`, `USER` and `USERNAME`
- ruby specific environment settings `GEM_HOME`, `GEM_PATH`
- jit specific environment settings `JIT_HOME` (See: [https://github.com/radiospiel/jit](https://github.com/radiospiel/jit))
- kinko specific environment settings `KINKO_HOME`

The PATH is set to include 

- the kinko application's *bin, var/bin,* and *var/gems/bin* directories
- kinko's *bin* and *sbin* directories
- a default set: `/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin`

Note: The following is an example installation command:

    sudo install -o root -g kinko-test-server -m 4510 bin/run-as bin/kinko-test-server

# run-as examples

Consider the following scenario:

## interfacing via scripts

There is one application, which is managed by a `kinko-server` user. This application
provides two public interfaces via scripts: `read-server` and `write-server`. 
The `read-server` script reads the content of the server status, while the `write-server`
script writes the server status.

Only some users should be allowed to use `read-server`, and a different set of users
should be allowed to use `write-server`.

This scenario can be managed using the following implementation:

1. create a script `kinko-read-server` with this content:

        #!/bin/sh
        exec kinko-server read-server
    
    and install it under the name 
