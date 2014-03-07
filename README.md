# Some tools for the kinko base system

## Account management

### mkgroups

    mkgroups group ..

A shell script which creates one or more groups. 

### mkuser

    mkuser user [ group .. ]

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
