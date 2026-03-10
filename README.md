# Sistem distribuit de comunicare criptată folosind AES-128

## Descriere
Acest proiect implementează un sistem distribuit de comunicare între mai multe stații folosind socket-uri TCP. 
Mesajele și fișierele transmise între stații sunt criptate folosind algoritmul simetric AES-128.

Sistemul permite:
- transmiterea de mesaje criptate între stații
- transferul de fișiere criptate
- decriptarea și refacerea fișierelor la destinație

Implementarea algoritmului AES este realizată manual în C++, fără utilizarea bibliotecilor externe de criptografie.

## Tehnologii utilizate
- Limbaj: C++
- Protocol de transport: TCP
- Comunicare: Socket-uri
- Algoritm de criptare: AES-128
- Topologie rețea: Stea (Client - Server)

## Arhitectura sistemului
Sistemul folosește o arhitectură client-server într-o topologie de tip stea.
Serverul gestionează conexiunile și redirecționează mesajele între stații.

Clientii:
- criptează mesajele folosind AES-128
- trimit datele prin socket TCP
- decriptează mesajele primite

## Fluxul de criptare
1. Utilizatorul introduce un mesaj sau selectează un fișier
2. Datele sunt criptate folosind AES-128
3. Mesajul criptat este trimis prin TCP socket
4. Destinatarul primește datele
5. Datele sunt decriptate folosind aceeași cheie

## Transfer de fișiere
Pentru transferul de fișiere:

1. Fișierul este împărțit în blocuri
2. Fiecare bloc este criptat cu AES
3. Blocurile criptate sunt trimise prin rețea
4. La destinație blocurile sunt decriptate
5. Fișierul original este reconstruit

## Funcționalități
- criptare AES-128
- comunicare TCP între stații
- transfer de mesaje criptate
- transfer de fișiere criptate
- reconstrucția fișierelor la destinație
