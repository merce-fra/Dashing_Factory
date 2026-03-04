
# --------------
# PREREQUISITES
# --------------
# The implementation requires the existence of a full functioning ns-3 installation, 
# ideally using the version 3.37

# -------------------------
# INSTALATION: STEPS 1->7
# ------------------------------------

# 1/ Move to the ns-3's root-directory.
cd <ns3_root_dir>

# 2/ Add the content of contrib/, scratch/ directories 
#    to the correspondant folders of the ns-3's installation.
cp  contrib/* ./contrib
cp  scratch/* ./scratch

# 3/ Add the content of ./src/internet/model/  
#    to the same location into the ns-3's root-directory.
cp ./src/internet/model/* ./src/internet/model

# 4/ Copy the folders docs/, _bash_scripts/_and scripts/ 
#    into the ns-3's root-directory.
cd -R docs bash_scripts scripts ./

# 5/ Create a folder named outputs/ at the ns-3's root-directory.
mkdir ./outputs

# 6/ From the ns-3 root-dir execute 
./ns3 configure --enable-examples

# 7/ ns-3 compiles with ./ns3.
./ns3

