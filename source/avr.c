#include "hd44780.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define ROZMIAR 16


//--------------------------------
// Wyswietlacz
//--------------------------------
void WYSW_EkranStartowy(void);
void WYSW_WybierzTryb(void);


//--------------------------------
// Klawiatura
//--------------------------------
void KLAW_Konfiguracja(void);
char KLAW_OdczytajZnak(void);
char KLAW_KonwertujZnak(char);


//--------------------------------
// Bufor
//--------------------------------
char * bufor[ROZMIAR] = {0};
int bufor_rozmiar = 0;

void BUF_Wyczysc(void);
void BUF_Wpisz(char);
void BUF_Cofnij(void);


//--------------------------------
// Stos
//--------------------------------
char * stos[ROZMIAR] = {0};
char znak[ROZMIAR];
int stos_rozmiar = 0;

void STOS_Wyczysc(void);
void STOS_Wrzuc(char *);
char * STOS_Sciagnij(void);


//--------------------------------
// Wyjscie
//--------------------------------
char * wyjscie[ROZMIAR] = {0};
int wyjscie_rozmiar = 0;

void WYJ_Wyczysc(void);
void WYJ_Wpisz(char *);


//--------------------------------
// Odwrotna Notacja Polska
//--------------------------------
void ONP_Translacja(void);
void ONP_WyswietlTranslacje(void);
void ONP_Wartosciowanie(void);
void ONP_WyswietlWartosciowanie(void);
void ONP_BuforNaWyjscie(void);


int main(void)
{
	char znak = 0;
	int tryb_klas = 0;
	int tryb_onp = 0;

	LCD_Initalize();
	KLAW_Konfiguracja();
	WYSW_EkranStartowy();
	LCD_Clear();
	for (;;)
	{
		WYSW_WybierzTryb();
		if ((znak = KLAW_OdczytajZnak()) > 0)
		{
			if (znak == 1) tryb_klas = 1;
			if (znak == 6) tryb_onp = 1;
		}
		if (tryb_klas || tryb_onp)
		{
			LCD_Clear();
			if (tryb_klas)
			{
				for (;;)
				{
					if ((znak = KLAW_OdczytajZnak()) > 0)
					{
						if (znak != 1 && znak != 6 && znak != 11 && znak != 16)
							BUF_Wpisz(KLAW_KonwertujZnak(znak));
						if (znak == 11)
							BUF_Cofnij();
						if (znak == 16)
						{
							ONP_Translacja();
							ONP_Wartosciowanie();
							LCD_Clear();
							for (;;)
							{
								ONP_WyswietlTranslacje();
								ONP_WyswietlWartosciowanie();
								if ((KLAW_OdczytajZnak()) == 11)
									break;
							}
							break;
						}
					}
				}
			}
			if (tryb_onp)
			{
				for (;;)
				{
					if ((znak = KLAW_OdczytajZnak()) > 0)
					{
						if (znak != 1 && znak != 6 && znak != 11 && znak != 16 && znak != 18 && znak != 19)
							BUF_Wpisz(KLAW_KonwertujZnak(znak));
						if (znak == 19)
							BUF_Wpisz('|');
						if (znak == 11)
							BUF_Cofnij();
						if (znak == 16)
						{
							ONP_BuforNaWyjscie();
							ONP_Wartosciowanie();
							LCD_Clear();
							for (;;)
							{
								ONP_WyswietlWartosciowanie();
								if ((KLAW_OdczytajZnak()) == 11)
									break;
							}
							break;
						}
					}
				}
			}
			LCD_Clear();
			BUF_Wyczysc();
			STOS_Wyczysc();
			WYJ_Wyczysc();
			znak = 0;
			tryb_klas = 0;
			tryb_onp = 0;
		}
	}
	return 0;
}

void WYSW_EkranStartowy()
{
	char tytul[] = "KALKULATOR ONP";
	int i;

	LCD_GoTo(2, 0);
	LCD_WriteText("Dawid Myslak");
	_delay_ms(300);
	LCD_GoTo(1, 1);
	for (i = 0; i < 14; i++)
	{
		LCD_WriteData(tytul[i]);
		_delay_ms(150);
	}
	_delay_ms(1200);
}

void WYSW_WybierzTryb()
{
	LCD_GoTo(2, 0);
	LCD_WriteText("WYBIERZ TRYB");
	LCD_GoTo(4, 1);
	LCD_WriteText("KLAS/ONP");
}

void KLAW_Konfiguracja()
{
	DDRD  = 0x0f;
	PORTD = 0xff;
	DDRB  &= 0xfe;
	PORTB |= 1;
}

char KLAW_OdczytajZnak()
{
	char wiersz, kolumna, znak;

	for (wiersz = 0xfe, znak = 1; wiersz >= 0xf7; wiersz = (wiersz << 1 | 0x1) & 0xff)
	{
		PORTD = wiersz; 
		for (kolumna = 0x10; kolumna > 0x00; kolumna <<= 1, znak++)
			if (!(PIND & kolumna)) return znak;
		PORTB |= 1;
		if (!(PINB & 1)) return znak;
		znak++;
	}	
	return 0;
}

char KLAW_KonwertujZnak(char znak)
{
	switch (znak)
	{
		case 2:
			return '/';
		case 3:
			return '9';
		case 4:
			return '8';
		case 5:
			return '7';
		case 7:
			return '*';
		case 8:
			return '6';
		case 9:
			return '5';
		case 10:
			return '4';
		case 12:
			return '-';
		case 13:
			return '3';
		case 14:
			return '2';
		case 15:
			return '1';
		case 17:
			return '+';
		case 18:
			return ')';
		case 19:
			return '(';
		case 20:
			return '0';
	}
	return 0;
}

void BUF_Wyczysc()
{
	int i;
	
	for (i = 0; i < bufor_rozmiar; i++)
	{
		free(bufor[i]);
		bufor[i] = 0;
	}
	bufor_rozmiar = 0;
}

void BUF_Wpisz(char znak)
{

	char * nowy;

	if (bufor_rozmiar < ROZMIAR)
	{
		nowy = (char *)malloc(sizeof(char) * 2);
		nowy[0] = znak;
		nowy[1] = '\0';
		bufor[bufor_rozmiar] = nowy;
		bufor_rozmiar++;
		LCD_WriteData(znak);
		_delay_ms(300);
	}
}

void BUF_Cofnij()
{
	if (bufor_rozmiar > 0)
	{
		bufor_rozmiar--;
		free(bufor[bufor_rozmiar]);
		bufor[bufor_rozmiar] = 0;
		LCD_GoTo(bufor_rozmiar, 0);
		LCD_WriteData(' ');
		LCD_GoTo(bufor_rozmiar, 0);
		_delay_ms(300);
	}
}

void STOS_Wyczysc()
{
	int i;
	
	for (i = 0; i < stos_rozmiar; i++)
	{
		free(stos[i]);
		stos[i] = 0;
	}
	stos_rozmiar = 0;
}

void STOS_Wrzuc(char * znak)
{
	char * nowy;

	if (stos_rozmiar < ROZMIAR)
	{
		nowy = (char *)malloc(sizeof(char) * (strlen(znak) + 1));
		strcpy(nowy, znak);
		stos[stos_rozmiar] = nowy;
		stos_rozmiar++;
	}
}

char * STOS_Sciagnij()
{
	if (stos_rozmiar > 0)
	{
		stos_rozmiar--;
		strcpy(znak, stos[stos_rozmiar]);
		free(stos[stos_rozmiar]);
		stos[stos_rozmiar] = 0;
	}
	return znak;
}

void WYJ_Wyczysc()
{
	int i;
	
	for (i = 0; i < wyjscie_rozmiar; i++)
	{
		free(wyjscie[i]);
		wyjscie[i] = 0;
	}
	wyjscie_rozmiar = 0;
}

void WYJ_Wpisz(char * znak)
{
	char * nowy;

	if (wyjscie_rozmiar < ROZMIAR)
	{
		nowy = (char *)malloc(sizeof(char) * (strlen(znak) + 1));
		strcpy(nowy, znak);
		wyjscie[wyjscie_rozmiar] = nowy;
		wyjscie_rozmiar++;
	}
}

void ONP_Translacja()
{
	char liczba[ROZMIAR];
	int i, j;
	
	for (i = 0; i < bufor_rozmiar; i++)
	{
		if (!strcmp(bufor[i], "+"))
		{
			if (stos_rozmiar == 0 || !strcmp(stos[stos_rozmiar - 1], "("))
				STOS_Wrzuc(bufor[i]);
			else
			{
				while (stos_rozmiar != 0 && strcmp(stos[stos_rozmiar - 1], "("))
					WYJ_Wpisz(STOS_Sciagnij());
				STOS_Wrzuc(bufor[i]);
			}
		}
		else if (!strcmp(bufor[i], "-"))
		{
			if (stos_rozmiar == 0 || !strcmp(stos[stos_rozmiar - 1], "("))
				STOS_Wrzuc(bufor[i]);
			else
			{
				while (stos_rozmiar != 0 && strcmp(stos[stos_rozmiar - 1], "("))
					WYJ_Wpisz(STOS_Sciagnij());
				STOS_Wrzuc(bufor[i]);
			}
		}
		else if (!strcmp(bufor[i], "*"))
		{
			if (stos_rozmiar == 0 || !strcmp(stos[stos_rozmiar - 1], "(") || !strcmp(stos[stos_rozmiar - 1], "+") || !strcmp(stos[stos_rozmiar - 1], "-"))
				STOS_Wrzuc(bufor[i]);
			else
			{
				while (stos_rozmiar != 0 && strcmp(stos[stos_rozmiar - 1], "(") && !strcmp(stos[stos_rozmiar - 1], "+") && !strcmp(stos[stos_rozmiar - 1], "-"))
					WYJ_Wpisz(STOS_Sciagnij());
				STOS_Wrzuc(bufor[i]);
			}
		}
		else if (!strcmp(bufor[i], "/"))
		{
			if (stos_rozmiar == 0 || !strcmp(stos[stos_rozmiar - 1], "(") || !strcmp(stos[stos_rozmiar - 1], "+") || !strcmp(stos[stos_rozmiar - 1], "-"))
				STOS_Wrzuc(bufor[i]);
			else
			{
				while (stos_rozmiar != 0 && strcmp(stos[stos_rozmiar - 1], "(") && !strcmp(stos[stos_rozmiar - 1], "+") && !strcmp(stos[stos_rozmiar - 1], "-"))
					WYJ_Wpisz(STOS_Sciagnij());
				STOS_Wrzuc(bufor[i]);
			}
		}
		else if (!strcmp(bufor[i], "("))
			STOS_Wrzuc(bufor[i]);
		else if (!strcmp(bufor[i], ")"))
		{
			while (strcmp(stos[stos_rozmiar - 1], "("))
				WYJ_Wpisz(STOS_Sciagnij());
			STOS_Sciagnij();
		}
		else
		{
			strcpy(liczba, bufor[i]);
			j = i + 1;
			while (bufor[j] != 0 && strcmp(bufor[j], "+") && strcmp(bufor[j], "-") && strcmp(bufor[j], "*") && strcmp(bufor[j], "/") && strcmp(bufor[j], "(") && strcmp(bufor[j], ")"))
			{
				strcat(liczba, bufor[j]);
				j++;
			}
			i = j - 1;
			WYJ_Wpisz(liczba);
		}
	}
	while (stos_rozmiar > 0)
		WYJ_Wpisz(STOS_Sciagnij());
}

void ONP_WyswietlTranslacje()
{
	int i;
	
	LCD_GoTo(0, 0);
	for (i = 0; i < wyjscie_rozmiar; i++)
	{
		LCD_WriteText(wyjscie[i]);
		LCD_WriteData('|');
	}
}

void ONP_Wartosciowanie()
{
	char znak[ROZMIAR], znak1[ROZMIAR], znak2[ROZMIAR];
	int i, wynik, liczba1, liczba2;
	
	for (i = 0; i < wyjscie_rozmiar; i++)
	{
		if (!strcmp(wyjscie[i], "+"))
		{
			strcpy(znak1, STOS_Sciagnij());
			strcpy(znak2, STOS_Sciagnij());
			liczba1 = atoi(znak1);
			liczba2 = atoi(znak2);
			wynik = liczba2 + liczba1;
			STOS_Wrzuc(itoa(wynik, znak, 10));
		}
		else if (!strcmp(wyjscie[i], "-"))
		{
			strcpy(znak1, STOS_Sciagnij());
			strcpy(znak2, STOS_Sciagnij());
			liczba1 = atoi(znak1);
			liczba2 = atoi(znak2);
			wynik = liczba2 - liczba1;
			STOS_Wrzuc(itoa(wynik, znak, 10));
		}
		else if (!strcmp(wyjscie[i], "*"))
		{
			strcpy(znak1, STOS_Sciagnij());
			strcpy(znak2, STOS_Sciagnij());
			liczba1 = atoi(znak1);
			liczba2 = atoi(znak2);
			wynik = liczba2 * liczba1;
			STOS_Wrzuc(itoa(wynik, znak, 10));
		}
		else if (!strcmp(wyjscie[i], "/"))
		{
			strcpy(znak1, STOS_Sciagnij());
			strcpy(znak2, STOS_Sciagnij());
			liczba1 = atoi(znak1);
			liczba2 = atoi(znak2);
			wynik = liczba2 / liczba1;
			STOS_Wrzuc(itoa(wynik, znak, 10));
		}
		else
			STOS_Wrzuc(wyjscie[i]);
	}
}

void ONP_WyswietlWartosciowanie()
{
	LCD_GoTo(0, 1);
	LCD_WriteText("WYNIK ");
	LCD_WriteText(stos[0]);
}

void ONP_BuforNaWyjscie()
{
	char liczba[ROZMIAR];
	int i, j;
	
	for (i = 0; i < bufor_rozmiar; i++)
	{
		if (!strcmp(bufor[i], "+") || !strcmp(bufor[i], "-") || !strcmp(bufor[i], "*") || !strcmp(bufor[i], "/"))
			WYJ_Wpisz(bufor[i]);
		else if (!strcmp(bufor[i], "|"))
			;
		else
		{
			strcpy(liczba, bufor[i]);
			j = i + 1;
			while (bufor[j] != 0 && strcmp(bufor[j], "+") && strcmp(bufor[j], "-") && strcmp(bufor[j], "*") && strcmp(bufor[j], "/") && strcmp(bufor[j], "|"))
			{
				strcat(liczba, bufor[j]);
				j++;
			}
			i = j - 1;
			WYJ_Wpisz(liczba);
		}
	}
}