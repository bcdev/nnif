cd ../data/v5_00.6
../../../nnftool-1.3/bin/nnftool -ffbpx -is1-3 57.29577951308232 -n -b -o case2.nnf 60x20x5_639.4.net 15x20_144.8.net 0.6 processCase2Net
../../../nnftool-1.3/bin/nnftool -o case2.nna case2.nnf
../../bin/case2 case2.nnf case2_gkss.inp case2_nnf.out
cd ../../bin
