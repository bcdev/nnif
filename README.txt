________________________________________________________________

 System requirements
________________________________________________________________


For reading and writing NNs in binary format a 32-bit big-endian
architecture is required.

________________________________________________________________

 MERIS NN releated directories
________________________________________________________________


Project sub-directories:

	case2nn      - Schiller's NNs
	ctpnn        - FUB's NNs
	nnftool      - BC's nnftool for net generation
	nnif         - BC's NN API (for ESA MERIS Processor)


________________________________________________________________

 TODOs for new FUB NNs 
________________________________________________________________

1) go to ctpnn and type "ls" or "l"
2) create new data directory, "mkdir ctp_v<n>"
   where <n> is the last version number plus one.
3) "cd ctp_v<n>" and copy the FUB data including the *.nna files 
   into it. Note (for FTP transfer) that the *.nna file are 
   plain text.
4) Convert the *.nna files into binary *.nnf files. Type
   " ../../nnftool-1.3/bin/nnftool -b all_na_old.nna" and you'll
   get the desired "all_na_old.nnf".

________________________________________________________________

 TODOs for new Schiller NNs
________________________________________________________________


1) go to case2nn/data/. and type "ls" or "l"
2) create new Schiller NNs version directory, type "mkdir v<n>"
   where <n> is the last version number plus one.
3) Usually schiller NNs delivery consists of
    o inverse and forward NNs in Schiller format (*.net files)
    o a test input vector file (*.inp)
    o one or more test result files (*.res*)
   Copy all delivery files to v<n>.
4) Reformat the test input file:
   Multiply first 3 columns with PI/180 (to get radians)
   Take the EXP of the 8 following columns (to get reflectances)
   Store the new file under v<n>/case2_gkss.inp
5) Reformat the test output file:
   Take the EXP of the first 3 columns (to get concentrations)
   Let the 4th column unchanged (the quality flag)
   Store the new file under v<n>/case2_gkss.out
6) go to case2nn/bin and copy the last test_v<n-1>.sh
   to test_v<n>.sh and adapt the content accordingly:
   a) modify data directory to data/v<n>
   b) in the nnftool -ffbpx call modify arguments for inverse net, 
      forward net and threshold for flag creation
      (Dr. Schiller usually supplies the threshold via e-mail)
7) execute script, type "source test_v<n>.sh"

________________________________________________________________

 Modifying nnftool
________________________________________________________________

1) go to nnftool/src
2) if new features are shall added, create nnftool backup by
   copying nnftool.c to nnftool-<old version>.c
3) open nnftool.c in editor
4) increase internal version number according to the
   changes to me made (bug-fix or feature-add)
5) make your changes
6) save nnftool.c
7) go to nnftool/.
8) type "make release"
9) in the case of success, type "cp ./build/release/nnftool ./bin"
