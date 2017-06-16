Procesor:
ATMega 328 20MHz
Okres taktu procesora: 0,05us - 50ns

Wykorzystane peryferia:

	timer1 (16bitowy) jako odniesienie czasu
	I2C jako interfejs komunikacji dla DS1307, freq 100 kHz
	przerwania PCINT grup 0 i 1 (S0, S1, czujnik Halla i sygna³ zmiany sekund z DS)
	
S0 - tryb jednorazowego wciœniêcia (reaguje z opuŸnieniem na zbocze narastaj¹ce)
S1 - tryb szybkiego narastania podczas przytrzymania

Bajt portu diod to zawartosc wyswietlacza od najmlodszego do najstarszego bitu (od góry do dolu patrz¹c
na pracujace urzadzenie). Obroty silnika w prawo (od strony silnika), pocz¹tek danych wyœwietlany od
pozycji magnesu.
  
Obliczenia konfiguracyjne timera1 (odniesienia czasu):
Max predkosc obrotowa: 2000 obr/min ~34 obr/s
Okres jednego obrotu: 29,4 ms = 29 400 us
Podzia³ka czasowa obrotu: 1000
Wymagany okres wyzwolenia licznika: <= 29,9 us; przyjêto 30 us
Iloœc taktow zegara na wyzwolenie: <= 588; przyjêto 600
Preskaler: 1
OCR1A: 600

 Alfabet:
 Litery du¿e - {0 - 25} 
 Cyfry - {26 - 35} 
 Litery ma³e - {36 - 61}
 Error - 62
 . - 63
 , - 64
 ! - 65
 ? - 66
 spacja - 67

// niedokonczony opis, patrz do kodu