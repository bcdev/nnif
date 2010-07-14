cd ../data/v2
../../../nnftool-1.3/bin/nnftool -ffbpx -is1-3 57.29577951308232 -n -b -o case2.nnf 30x20x10x5_796.4.net 15x20_592.1.net 1.0 processCase2Net
../../../nnftool-1.3/bin/nnftool -o case2.nna case2.nnf
../../bin/case2 case2.nnf case2_gkss.inp case2_nnf.out
cd ../../bin
