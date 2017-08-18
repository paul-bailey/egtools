bin_SCRIPTS = \
 license \
 bundle \
 fwrd \
 gfwrd \
 mandelbrot \
 fold \
 backup \
 egcify \
 acclean
if BUILD_PLP
bin_SCRIPTS += plp
endif
if BUILD_PRINTIT
bin_SCRIPTS += printit
endif
if BUILD_MATH
bin_SCRIPTS += \
 egnum \
 egadd \
 egmul \
 egdiv \
 egsub
endif

eg_intermed = \
 egadd.sh \
 egmul.sh \
 egdiv.sh \
 egsub.sh

fwrd_intermed = \
 fwrd.sh \
 gfwrd.sh

$(fwrd_intermed): makefwrd.sh
	sh makefwrd.sh

$(eg_intermed): mkmath.sh
	sh mkmath.sh

# Would love to add $(eval...$(foreach...$(bin_SCRIPTS)..., but doesn't
# work everywhere
license:license.sh
bundle:bundle.sh
fwrd:fwrd.sh
gfwrd:gfwrd.sh
mandelbrot:mandelbrot.sh
plp:plp.sh
printit:printit.sh
egnum:egnum.sh
egadd:egadd.sh
egmul:egmul.sh
egdiv:egdiv.sh
egsub:egsub.sh
fold:fold.sh
acclean:acclean.sh
backup:backup.sh
egcify:egcify.sh

# TODO: POSIXive version of
#     DATE = $(shell date +"%B %d, %y %T")
#  This is not as ideal...
DATE = `date +"%B %d, %y %T"`

bundle fwrd gfwrd plp printit acclean egnum egadd egmul egdiv egsub fold:
	sh build.sh "/bin/sh" "$(DATE)" $< "$(VERSION)" > $@
	chmod +x $@

mandelbrot license:
	sh build.sh "/usr/bin/env bash" "$(DATE)" $< "$(VERSION)" > $@
	chmod +x $@

backup egcify:
	cp $< $@
	chmod +x $@

CLEANFILES = \
 $(bin_SCRIPTS) \
 $(fwrd_intermed) \
 $(eg_intermed)

EXTRA_DIST = \
 mandelbrot.sh \
 build.sh \
 bundle.sh \
 egnum.sh \
 plp.sh \
 printit.sh \
 makefwrd.sh \
 acclean.sh \
 fold.sh \
 egcify.sh \
 mkmath.sh
