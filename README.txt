
Warning: All documentation in this project is in portuguese.

LNIDB
(C) 2010-2017 Felipe P.G. Bergo <fbergo at gmail dot com>

LNIDB é um sistema PACS (banco de dados de imagens médicas)
desenvolvido entre 2010 e 2017 para as necessidades específicas do
Laboratório de Neuroimagem da FCM-Unicamp.

Como não mantenho mais o sistema, estou disponibilizando o código
sob licença BSD-like.

LNIDB roda em servidores Linux, e está escrito em PHP, Perl e Java, e
usa o PostgreSQL como backend de banco de dados. Ele foi originalmente
desenvolvido em um servidor com Fedora Linux 13 e posteriormente
transferido para sistemas com Fedora 14 e CentOS 6. Embora seja
possível usá-lo em distribuições mais recentes, as instruções para
instalação dos serviços em sistemas mais novos baseados em systemd
devem diferir um pouco.

O código-fonte e os arquivos de configuração provavelmente contêm IPs
da rede interna do LNI (143.106.129.42 era o IP do servidor original,
143.106.129.0/26 a sub-rede). A instalação em redes diferentes deve
exigir atenção com os endereços.

Diretórios:

lnidb-backend: contém a parte que roda no servidor, não exposta ao
usuário.

Os principais elementos são:
 lnidb.sql    - esquema de banco de dados
 lnidbtask.pl - daemon que processa a fila do sistema, escrito em Perl.
 lnidbtask.sh - script init.d para automação do início/parada do dameon.

 diversos arquivos de configuração para iptables, httpd/apache, servidor
 de FTP

lnidb-backend/diva: código-fonte em Java do applet de visualização que
 roda nos clientes. Como a maioria dos browsers atuais não aceita
 mais applets Java, tornou-se um problema para o sistema.

lnidb-backend/binutils: executáveis usados pelo backend
 perldbr.pl : código meu, Perl)
 dicom2scn, scn2ana, scntool, splitmri: código em https://github.com/fbergo/braintools
 nii2mnc, mincreshape: parte no MNI MINC (provavelmente versão 1.5.x)
 dcm2nii: utilitário do MRicron.

lnidb-backend/scripts: scripts em Perl para manutenção do backend.

lnidb-web: contém a interface web exposta aos usuários, escrita em PHP e Javascript.

lnidb-frontend: contém um mock-up de um cliente Java stand-alone para
o sistema (bem incompleto e não-utilizável).

As instruções de instalação do sistema então em lnidb-backend/README

Novembro, 2017.

