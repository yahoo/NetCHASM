rm -rf demoCA
rm *.csr
rm *.key
rm *.crt
mkdir -p demoCA
mkdir -p demoCA/certs
mkdir -p demoCA/crl
mkdir -p demoCA/newcerts
touch demoCA/index.txt
touch demoCA/serial
echo "1000" > demoCA/serial
echo 123456 > pass.txt
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/oath/openssl/1.1.1/lib
export LD_LIBRARY_PATH
cnfFile=$1/openssl.cnf
openssl req -new -x509 -batch -passout file:pass.txt -keyout ca.key -out ca.crt -config ${cnfFile}
openssl genrsa -aes128 -passout file:pass.txt -out server.key 1024
openssl rsa -passin file:pass.txt -in server.key -out server.key
openssl req -new -batch -key server.key -out server.csr -config ${cnfFile} -subj "/CN=healthmon_server.com"
openssl ca -batch -passin file:pass.txt -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config ${cnfFile}
openssl genrsa -aes128 -passout file:pass.txt -out client.key 1024
openssl rsa -passin file:pass.txt -in client.key -out client.key
openssl genrsa -aes128 -passout file:pass.txt -out client_mismatch.key 1024
openssl rsa -passin file:pass.txt -in client_mismatch.key -out client_mismatch.key
openssl req -new -batch -key client.key -out client.csr -config ${cnfFile} -subj "/CN=healthmon_client.com"
openssl req -new -batch -key client.key -out client_self.csr -config ${cnfFile} -subj "/CN=healthmon_client_self.com"
openssl ca -batch -passin file:pass.txt -in client.csr -out client.crt -cert ca.crt -keyfile ca.key -config ${cnfFile}
openssl x509 -req -in client_self.csr -signkey client.key -out client_self.crt
