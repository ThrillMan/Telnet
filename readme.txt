Domyslny port = "1234"
Domyslne IP = "192.168.56.102"
Wpisane tutaj domyslne ip to adres karty sieciowej jaki ustawilem na mojej maszynie wirtualnej z serwerem

Po uzupełnieniu(lub nie) informacji o porcie i IP użytkownik może podłączyć się do serwera telnet.

Server kompilujemy wpisując gcc -Wall server.c client_handler.c -o server
uruchamiamy wpisując ./server

GUI aplikacji klienta zbudowane jest przy pomocy TKinter
Do komunikacji z serwerem stworzone zostały dwa wątki, jeden do odczytu drugi do przesyłu danych.

Z powodu nie działającej u mnie funkcji exec(na różnych wirtualnych maszynach)
postanowiłem użyć funkcji popen i zbudować w okół niej wrapper.

Dodałem funkcje które umożliwiają zmiane katalogu, czyszczenie ekranu oraz obsługę błędów
(np błędnie wpisana komenda, albo brak uprawnień)

Jako iż jako klient gdy się podłączymy do serwera nie mamy wielu uprawnień, tak też
domyślnym katalogiem jest /var/tmp w którym możemy tworzyć pliki jakie tylko chcemy.

Program mój pozwala na podstawową edycję i plików i wyświetlanie ich.
Nie pozwala na obsługę programów i wpisanie uprawnień administratora. 

