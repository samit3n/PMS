/*
 * =====================================================================================
 *
 *       Filename:  pms.cpp
 *
 *    Description:  Pipeline Merge Sort implementation
 *
 *        Version:  1.0
 *        Created:  10/19/2016 01:00:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Vojtech Dvoracek, xdvora0y@stud.fit.vutbr.cz
 *   Organization:  FIT VUT BRNO 
 *
 * =====================================================================================
 */



#include <iostream>
#include <fstream>
#include <mpi.h>
#include <vector>
#include <cmath>

//#define DEBUG
const int DBG = 0;

#ifdef DEBUG
#define DBG(x) cout << x;
#else
#define DBG(x)
#endif


const int QTOP = 0;
const int QBOTTOM = 1;
const int MSGTAG = 0;


//message stuff
const int DATA = 0;
const int QUE = 1;
const int TICK = 2;
const int EON = 255;  //end of numbers message

using namespace std;

void printVect(vector<unsigned char > *vect)
{
    for(vector<unsigned char>::iterator it = vect->begin(); it != vect->end(); ++it){
        cout << (int)*it;
        if (it != vect->end())
            cout << " ";
    }

    cout << endl;

}

void printMsg(unsigned msg[])
{
    if(DBG)
    {
        cout << "Data: " << msg[DATA];
        cout << " Queue: " << msg[QUE] << endl;
    }
    
}

int main(int argc, char * argv[])
{
    int numprocs, myId;             //COMM WORLD INFO
    vector<unsigned char> qT,qB;    //queues

    MPI_Init(&argc, &argv);
    MPI_Status mpstat;
    ifstream numbers;               //input queue (file)
    unsigned cpuTick = 0;

    unsigned tick = 0;
   

   MPI_Comm_size(MPI_COMM_WORLD,&numprocs);  //count running processes
   MPI_Comm_rank(MPI_COMM_WORLD,&myId);      //rank this process by number
   

   if(myId == 0)
   {
       //first processor behaivor
       
       //input file reading
       numbers.open("numbers", ios::in|ios::binary|ios::ate);

       unsigned fSize;
       fSize =numbers.tellg();
       numbers.seekg(0, ios::beg);
      
        //reading input 
       vector<unsigned char> input((std::istreambuf_iterator<char> (numbers)), 
                        (std::istreambuf_iterator<char>()));


       printVect(&input);

       unsigned int msg[3]; 

       //0 - number
       //1 - queue number QTOP,QBOTTOM | 255 END OF NUMBERS
       //2 = tick

       unsigned qNum = QTOP;     

       while(!input.empty()){
        //sending numbers to proc 
            msg[DATA] = input.front();
            input.erase(input.begin());
            msg[QUE] = qNum ;
            msg[TICK] = cpuTick;

            MPI_Send(&msg, 3, MPI_UNSIGNED, (myId+1), MSGTAG, MPI_COMM_WORLD);

            cpuTick++;
            qNum = ( msg[QUE] == QTOP ? QBOTTOM : QTOP );
            
       }         
        
       //end of input
       msg[QUE] = EON;
       MPI_Send(&msg, 3, MPI_UNSIGNED, (myId+1), MSGTAG, MPI_COMM_WORLD);
        
   }else if (myId == (numprocs -1))
   {    //last processor behavior
        
        unsigned int  recv[3];
        bool eon = false, start = false;
        int stCond = pow (2, myId - 1);
        


        while ( !eon || !qB.empty() || !qT.empty() ){ //main loop
              

            if (!eon){
                MPI_Recv(&recv, 3, MPI_UNSIGNED, (myId -1),MSGTAG,MPI_COMM_WORLD, &mpstat );
                DBG("P " << myId << " recvd ");
                printMsg(recv);

                
                if (recv[QUE] == QTOP)
                        qT.push_back( (unsigned char) recv[DATA]);
                if (recv[QUE] == QBOTTOM ) 
                        qB.push_back( (unsigned char) recv[DATA]);
                if (recv[QUE] ==  EON)
                        eon = true;
            }

            if( (!start and qT.size() >= stCond) and qB.size() >= 1)
                start = true;

            
            if(start){
                if ( !qB.empty() and !qT.empty() ){
                    if( qT.front() <= qB.front() ){
                        cout << (int) qT.front() << endl;
                        qT.erase(qT.begin());
                    }else{
                        cout << (int) qB.front() << endl;
                        qB.erase(qB.begin());
                    }
                
                }else if(!qB.empty()){
                    cout << (int) qB.front() << endl;
                    qB.erase(qB.begin());
                }else if (!qT.empty()){
                    cout << (int) qT.front() << endl;
                    qT.erase(qT.begin());
                }
           } 
        }

    }else{
    
       //other processors
       
        unsigned int  recv[3], msg[3];
        bool eon = false, start = false;
        unsigned cntSend = 0;
        int stCond = pow (2, myId - 1); //starting condition - top queue size
        int qSel = QTOP;         //switch based on formula
        int qSend, qPrev = -1;
        unsigned takeT = 0, takeB = 0, maxTake = stCond;
        bool stFlag = false;

        


        while ( !eon || !qB.empty() || !qT.empty() ){ //main loop
              

            if (!eon){
                MPI_Recv(&recv, 3, MPI_UNSIGNED, (myId -1),MSGTAG,MPI_COMM_WORLD, &mpstat );
                DBG("P " << myId <<  " recvd ");
                printMsg(recv);
                
                if (recv[QUE] == QTOP)
                        qT.push_back( (unsigned char) recv[DATA]);
                if (recv[QUE] == QBOTTOM ) 
                        qB.push_back( (unsigned char) recv[DATA]);
                if (recv[QUE] ==  EON)
                        eon = true;
            }

            if( (!start and qT.size() >= stCond) and qB.size() >= 1)
                  start = true;

            DBG("stFlag: " << stFlag << endl);
            DBG("P " << myId << " start " << start << " qT " << qT.size() << " qB " << qB.size() <<  endl);

            if (start and !stFlag){

                if( !qB.empty() and !qT.empty() ){

                    if( qT.front() <= qB.front() ){
                        msg[DATA] = qT.front();
                        qT.erase(qT.begin());
                        qSend = QTOP;
                    }else{
                        msg[DATA] = qB.front();
                        qB.erase(qB.begin());
                        qSend = QBOTTOM;
                    }
                }

                //sending actual lower number
                msg[QUE] = qSel;
                MPI_Send(&msg, 3, MPI_UNSIGNED, (myId+1), MSGTAG, MPI_COMM_WORLD);
                DBG("P " << myId << " send " );
                printMsg(msg);
                cntSend++;

                if(qSend == QTOP)
                    takeT++;
                else
                    takeB++;

                if(takeT == maxTake || takeB == maxTake)
                    stFlag = true;
                

           }else if(stFlag){
               
                
               if(takeT < maxTake){
                   msg[DATA] = qT.front();
                   qT.erase(qT.begin());
                   takeT++;
               }else if (takeB < maxTake){
                   msg[DATA] = qB.front();
                   qB.erase(qB.begin());
                   takeB++;
               }
               if(takeT == maxTake and takeB == maxTake){
                   takeT = 0;
                   takeB = 0;
                   stFlag = false;
               }

                                
               msg[QUE] = qSel;
               MPI_Send(&msg, 3, MPI_UNSIGNED, (myId+1), MSGTAG, MPI_COMM_WORLD);
               DBG("P " << myId << " send " );
               printMsg(msg);
               cntSend++;
                
          }
          
          //change target queue
          //using known formula to count elements, that should neighbor processor receive in first queue
          if( cntSend != 0 and  ((cntSend % ( (unsigned int)  pow (2,myId ))) == 0) ) 
                qSel = qSel == QTOP ? QBOTTOM : QTOP;
           
          //propagating End Of Numbers signal
          //CPU stops listening and only outputs all queues out 
          if (qT.empty() and qB.empty() and start){
              msg[QUE] = EON;
              MPI_Send(&msg, 3, MPI_UNSIGNED, (myId+1), MSGTAG, MPI_COMM_WORLD);
          }
        }
   }
   MPI_Finalize();

}
