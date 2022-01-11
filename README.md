# IKP_Projekat_A15
Projekat iz predmeta Industrijski komunikacioni protokoli u EES. 

A15. Replication

Potrebno je implementirati servis za replikaciju podataka. Servis implementira interface:

  ● RegisterService(int ServiceID)

  ● SendData(int ServiceID, void* data, int dataSize)

  ● Callback: void ReceiveData(void* data, int dataSize)

  ● Callback: void RequestIU(void* data, int dataSize)
  
Servis treba da implementira asinhronu replikaciju. Podaci koje servis dobija na slanje treba da smeste na
interni kružni red.

Prilikom uspostave veze, radi se poravnavanje sadržaja ovih servisa - integrity update (IU). Svaki od
Replikatora će kontaktirati registrovane procese koji će poslati svoje podatke odredišnoj strani. Nakon
završetka IU procedure, procesi će informacije slati korišćenjem SendData() funkcije.


Tim za izradu projekta: Nevena Panić PR43-2018 i Milan Stevanović PR128-2018
