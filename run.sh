#!/bin/bash

echo "ecoli, 3 tars, 1e03:"
./Compression -r ecoli/ecoli1_1e03.fna -t ecoli/ecoli2_1e03.fna ecoli/ecoli3_1e03.fna ecoli/ecoli4_1e03.fna
ls -l ecoli/ecoli2_1e03.fna
ls -l ecoli/ecoli3_1e03.fna
ls -l ecoli/ecoli4_1e03.fna
ls -l compress.zip
echo -e

echo "ecoli, 4 tars, 1e03:"
./Compression -r ecoli/ecoli1_1e03.fna -t ecoli/ecoli2_1e03.fna ecoli/ecoli3_1e03.fna ecoli/ecoli4_1e03.fna ecoli/ecoli5_1e03.fna
ls -l ecoli/ecoli2_1e03.fna
ls -l ecoli/ecoli3_1e03.fna
ls -l ecoli/ecoli4_1e03.fna
ls -l ecoli/ecoli5_1e03.fna
ls -l compress.zip
echo -e

echo -e "hiv, 2 tars, 1e04:"
./Compression -r hiv/hiv1_1e04.fna -t hiv/hiv2_1e04.fna hiv/hiv3_1e04.fna
ls -l hiv/hiv2_1e04.fna
ls -l hiv/hiv3_1e04.fna
ls -l compress.zip
echo -e

echo -e "hiv, 3 tars, 1e04:"
./Compression -r hiv/hiv1_1e04.fna -t hiv/hiv2_1e04.fna hiv/hiv3_1e04.fna hiv/hiv4_1e04.fna
ls -l hiv/hiv2_1e04.fna
ls -l hiv/hiv3_1e04.fna
ls -l hiv/hiv4_1e04.fna
ls -l compress.zip
echo -e

echo -e "hiv, 4 tars, 1e04:"
./Compression -r hiv/hiv1_1e04.fna -t hiv/hiv2_1e04.fna hiv/hiv3_1e04.fna hiv/hiv4_1e04.fna hiv/hiv5_1e04.fna
ls -l hiv/hiv2_1e04.fna
ls -l hiv/hiv3_1e04.fna
ls -l hiv/hiv4_1e04.fna
ls -l hiv/hiv5_1e04.fna
ls -l compress.zip
echo -e

echo -e "ecoli, 4 tars, 1e05:"
./Compression -r ecoli/ecoli1_1e05.fna -t ecoli/ecoli2_1e05.fna ecoli/ecoli3_1e05.fna ecoli/ecoli4_1e05.fna ecoli/ecoli5_1e05.fna
ls -l ecoli/ecoli2_1e05.fna
ls -l ecoli/ecoli3_1e05.fna
ls -l ecoli/ecoli4_1e05.fna
ls -l ecoli/ecoli5_1e05.fna
ls -l compress.zip
echo -e

echo -e "ecoli, 4 tars, 1e05, drugi podaci:"
./Compression -r ecoli/ecoli11_1e05.fna -t ecoli/ecoli22_1e05.fna ecoli/ecoli33_1e05.fna ecoli/ecoli44_1e05.fna ecoli/ecoli55_1e05.fna
ls -l ecoli/ecoli22_1e05.fna
ls -l ecoli/ecoli33_1e05.fna
ls -l ecoli/ecoli44_1e05.fna
ls -l ecoli/ecoli55_1e05.fna
ls -l compress.zip
echo -e

echo -e "ecoli, 2 tars, 1e06:"
./Compression -r ecoli/ecoli1_1e06.fna -t ecoli/ecoli2_1e06.fna ecoli/ecoli3_1e06.fna
ls -l ecoli/ecoli2_1e06.fna
ls -l ecoli/ecoli3_1e06.fna
ls -l compress.zip
echo -e