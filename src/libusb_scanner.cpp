//============================================================================
// Name        : Teste.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <usb.h>
#include <stdio.h>
#include <string.h>
using namespace std;

/*
    idVendor  0x04b4
    idProduct 0x0043
 */

#define USB_VENDOR 0x04b4
#define USB_PRODUCT 0x0043
#define TIMEOUT (5*1000)
#define EP2IN	0x02
#define MAXPACKETSIZE 512

/*
static struct usb_device *findsensorboard(uint16_t vendor, uint16_t product)
{
  struct usb_bus *bus;
  struct usb_device *dev;
  struct usb_bus *busses;

  usb_init();
  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();

  for (bus = busses; bus; bus = bus->next)
  {
    for (dev = bus->devices; dev; dev = dev->next)
    {
      if ((dev->descriptor.idVendor == vendor) && (dev->descriptor.idProduct == product))
      {
        return dev;
      }
    }
  }
  return NULL;
}*/


struct usb_bus *USB_init()
{
	usb_init();
	usb_find_busses();
	usb_find_devices();
	return(usb_get_busses());
}

/* Find USB device  */
struct usb_device *USB_find(struct usb_bus *busses, struct usb_device *dev)
{
	struct usb_bus *bus;
	for(bus=busses; bus; bus=bus->next)
	{
		for(dev=bus->devices; dev; dev=dev->next)
		{
			if( (dev->descriptor.idVendor==USB_VENDOR) && (dev->descriptor.idProduct==USB_PRODUCT) )
			{
				return( dev );
			}
		}
	}
	return( NULL );
}

/* USB Open */
struct usb_dev_handle *USB_open(struct usb_device *dev)
{
	struct usb_dev_handle *udev = NULL;

	udev=usb_open(dev);
	if( udev==NULL )
	{
		fprintf(stderr,"usb_open Error.(%s)\n",usb_strerror());
		exit(1);
	}

	if( usb_set_configuration(udev,dev->config->bConfigurationValue)<0 )
	{
		if( usb_detach_kernel_driver_np(udev,dev->config->interface->altsetting->bInterfaceNumber)<0 )
		{
			fprintf(stderr,"usb_set_configuration Error.\n");
			fprintf(stderr,"usb_detach_kernel_driver_np Error.(%s)\n",usb_strerror());
 		}
	}

	if( usb_claim_interface(udev,dev->config->interface->altsetting->bInterfaceNumber)<0 )
	{
		if( usb_detach_kernel_driver_np(udev,dev->config->interface->altsetting->bInterfaceNumber)<0 )
		{
			fprintf(stderr,"usb_claim_interface Error.\n");
			fprintf(stderr,"usb_detach_kernel_driver_np Error.(%s)\n",usb_strerror());
		}
	}

	return(udev);
}

/* USB Close */
void USB_close(struct usb_dev_handle *dh, struct usb_device *dev)
{
	if(usb_release_interface(dh, dev->config->interface->altsetting->bInterfaceNumber))
	{
		fprintf(stderr,"usb_release_interface() failed. (%s)\n",usb_strerror());
		usb_close(dh);
	}

	if( usb_close(dh)<0 )
	{
		fprintf(stderr,"usb_close Error.(%s)\n",usb_strerror());
	}
}

/* USB altinterface */
void USB_altinterface(struct usb_dev_handle *dh, int tyep, struct usb_device *dev)
{
	if(usb_set_altinterface(dh,tyep)<0)
	{
		fprintf(stderr,"Failed to set altinterface %d: %s\n", 1, usb_strerror());
		USB_close(dh, dev);
	}
}


int main() {
	//cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	printf("Procure o sensor handy ... \n");

	// Chame a função findKeyboard e armazene os resultados retornados.
	// A função leva o ID do fornecedor e o id do produto do teclado desejado como argumentos
	// para que saiba o que procurar
	struct usb_bus *bus = NULL;
	struct usb_device *dev = NULL;
	struct usb_dev_handle *dh = NULL;
	char buf[MAXPACKETSIZE];
	int ret = 0;

	memset(buf,0,MAXPACKETSIZE);

	usb_set_debug(255);

	/*-------------*/
	/* Initialize */
	/*-------------*/
	printf("starting!\n");
	bus=USB_init();
	dev=USB_find(bus,dev);
	if( dev==NULL )
	{
       printf("device not found\n");
	   exit(1);
	}
	printf("Initialize OK\n");


	/*-------------*/
	/* Device Open */
	/*-------------*/
	dh=USB_open(dev);
	if( dh==NULL )
	{
		exit(2);
	}

	char dname[100];

	ret = usb_get_driver_np(dh, dev->config->interface->altsetting->bInterfaceNumber, dname, 100); //get driver-name
	if (ret < 0)
	{
		fprintf(stderr,"Erro ao captura o nome do driver: %s\n", dname);
		if( usb_detach_kernel_driver_np(dh,dev->config->interface->altsetting->bInterfaceNumber)<0 )
		{
			fprintf(stderr,"usb_detach_kernel_driver_np Error.(%s)\n",usb_strerror());
 		}
		exit(3);
	}
	else
	if (ret == 0)
	{
		if( usb_detach_kernel_driver_np(dh,0)<0 )
		{
			fprintf(stderr,"usb_detach_kernel_driver_np Error.(%s)\n",usb_strerror());
 		}
	}

	fprintf(stderr,"Driver name: %s\n", dname);

	printf("Device Open OK\n");
	printf("Start Read data from EP2IN\n");

	ret = usb_bulk_read(dh, EP2IN, buf, sizeof(buf), TIMEOUT);
	if(ret < 0)
	{
		fprintf(stderr,"usb_bulk_read() failed? buf(%d) usb(%d)\n",(int)sizeof(buf), ret);
		fprintf(stderr,"usb_bulk_read error.(%s)\n",usb_strerror());
		USB_close(dh, dev);
		return 1;
	}

	printf("usb_bulk_read( %d ) finished\n", ret);
	printf("[ ");
	for (int i=0; i< MAXPACKETSIZE; i++)
	{
	    printf ("0x%02x, ", buf[i]);
	}
	printf(" ]\n");

	/*ret = 1;
	while (ret > 0)
	{
		ret = usb_bulk_read(dh, EP2IN, buf, sizeof(buf), TIMEOUT);
		if(ret < 0)
		{
			fprintf(stderr,"usb_bulk_read() failed? buf(%d) usb(%d)\n",(int)sizeof(buf), ret);
			fprintf(stderr,"usb_bulk_read error.(%s)\n",usb_strerror());
			USB_close(dh, dev);
			return 1;
		}
		else
		{
			printf("nbyte %d [ ", ret);
			for (int i=0; i < MAXPACKETSIZE; i++)
			{
				printf ("0x%02x, ", buf[i]);
			}
			printf(" ]\n\n");
		}
	}*/

	usb_resetep(dh, EP2IN);

	USB_close(dh, dev);
	printf("USB Fim\n");
	return 0;
}
