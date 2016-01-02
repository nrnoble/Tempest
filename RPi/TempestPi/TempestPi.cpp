
#include "stdafx.h"
#include <pthread.h>

#include "TempestCPU/TempestException.h"
#include "TempestIO/TempestIOStreamProxy.h"

#include "PiSerialStream.h"
#include "TempestPiIO.h"

#include "TempestPi.h"


TempestPi::TempestPi(void)
   :
      tempestRunner(&environment)
{
   // clear
   demo = false;
   terminated = false;
   monitorThread = 0;
}

TempestPi::~TempestPi(void)
{
   terminated = true;
   if (monitorThread != 0)
      pthread_join(monitorThread, NULL);
}

void TempestPi::Run(void)
{
   try
   {
      // start the monitor thread if it's not running
      if (monitorThread == 0)
      {
         int result = pthread_create(
            &monitorThread,
            NULL,
            &MonitorThreadEntry,
            this
            );
         if (result != 0)
            throw TempestException("Error creating Monitor thread");
         pthread_setname_np(monitorThread, "Monitor");
      }

       // create our peripherals
       TempestPiIO vectorIO;
       PiSerialStream serialStream;
       TempestIOStreamProxy soundIO(&serialStream);

       // create the runner object that drives the fake 6502
       tempestRunner.SetTempestIO(&soundIO, &vectorIO);
       if (demo)
         tempestRunner.SetDemoMode();

       // go
       tempestRunner.Start();

      // the IO object (i.e. the screen) takes over the main thread
      // from here
      vectorIO.Run();
   }
   catch (TempestException &te)
   {
      printf("%s\n", te.what());
   }
   catch (const char *s)
   {
      printf("%s\n", s);
   }
}

void TempestPi::MonitorThread(void)
{
   Log("Monitor running");

   while (!terminated)
   {
      // pause
      sleep(100);

      // look for any issues
      if (tempestRunner.IsTerminated())
      {
         Log("Tempest terminated");
         Log(tempestRunner.GetProcessorStatus().c_str());
         return;
      }
   }
}

void TempestPi::Log(const char *s)
{
   printf("%s\n", s);
}


void *TempestPi::MonitorThreadEntry(void *pThis)
{
   TempestPi *tempest = (TempestPi*)pThis;

   try
   {
      tempest->MonitorThread();
      tempest->Log("Monitor thread exit");
   }
   catch (...)
   {
      tempest->Log("Monitor thread unhandled exception");
   }

   return NULL;
}

