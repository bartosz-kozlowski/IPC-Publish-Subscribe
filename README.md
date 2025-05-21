### IPC Publish Subscribe
Celem projektu była implementacja systemu przekazywania (rozgłaszania) wiadomości do wszystkich procesów, które zasubskrybowały dany typ wiadomości (zarejestrowały się na ich odbiór). 
Tworzona aplikacja wykorzystuje mechanizm kolejek komunikatów. W ramach realizacji projektu konieczne było opracowanie protokołu komunikacyjnego i 2 programów: klienta i serwera. 
Każdy klient może wysyłać i otrzymywać wiadomości do/od pozostałych użytkowników systemu. W wymianie wiadomości pomiędzy klientami zawsze pośredniczy serwer (bezpośrednia komunikacja pomiędzy klientami jest zabroniona). 
Otrzymywane wiadomości wyświetlane są na ekranie.
