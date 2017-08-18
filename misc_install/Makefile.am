
svn_lpath = file://$(HOME)/Dropbox/hub
man_path = $(HOME)/share/man

install-data-local: myconfig emacs
	cp myconfig $(HOME)/myconfig
	cp emacs $(HOME)/.emacs
	echo "database \$$HOME/Dropbox/.todo" > $(HOME)/.todorc

install-data-hook:
	@echo "**************************************************************"
	@echo "* Be sure to add \"source myconfig\" to your bash setup script"
	@echo "**************************************************************"

noinst_SCRIPTS = myconfig

do_mkconfig = \
 sed -e "s,[@]SVN_LPATH[@],$(svn_lpath),g" \
     -e "s,[@]MAN_PATH[@],$(man_path),g" \
     -e "s,[@]PREFIX[@],$(prefix),g"

myconfig: myconfig_input
	$(do_mkconfig) $< > $@

CLEANFILES = myconfig
EXTRA_DIST = myconfig_input emacs
