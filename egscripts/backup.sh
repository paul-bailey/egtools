#!/bin/bash
outputdir=/Volumes/EGBackup/Backup
outputsvn=/Volumes/EGBackup/svnBackup
exclusions=exclusions.txt
svnbackupfile=svnhub_${stamp}.dump.gz

dir_ready=yes
if test ! -d $outputdir; then
    dir_ready=no
fi
if test ! -d $outputsvn; then
    dir_ready=no
fi
if test "x$dir_ready" = "xno"; then
    echo "Either $outputdir or $outputsvn do not exist yet" 1>&2
    echo "You must make it manually.  I dare not." 1>&2
    exit 1
fi
hname=`hostname | sed 's/\..*$//g'`
if test "x$hname" != xPauls-iMac; then
    echo "You're on the wrong computer, dummy!" 1>&2
    echo "Give someone the keys" 1>&2
    exit 1
fi
echo This will take a very long time. Good night.
cat > $exclusions <<\_AEOF
.CFUserTextEncoding
.Trash
.Xauthority
.android
.bash_sessions
.cache
.config
.cups
.dropbox
.dvdcss
.eclipse
.emme-cache
.lesshst
.local
.mailrc
.mplayer
.*_history
.rnd
.serverauth*
.subversion
.svn
.swo
.swp
.vim
.viminfo
.vimrc
.viper
.w3m
Applications
Backup
Desktop
Downloads
Dropbox
Library
Public
VirtualBox*
pennyshare
tmp
_AEOF
rsync --delete -arv --exclude-from=$exclusions $HOME $outputdir
svnadmin dump $HOME/Dropbox/hub | gzip > $svnbackupfile
mv $svnbackupfile $outputsvn
rm $exclusions
