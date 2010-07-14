cd ../data/v7_02.0
../../../nnftool-1.3/bin/nnftool -ffbpx -is1-3 57.29577951308232 -n -b -o case2.nnf 25x20x15x10x5_4018.3.net 15x20_365.5.net 2.0 processCase2Net
../../../nnftool-1.3/bin/nnftool -o case2.nna case2.nnf
../../bin/case2 case2.nnf case2_gkss.inp case2_nnf.out
cd ../../bin
