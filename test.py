
import RPi.GPIO as GPIO
import time
import socket, sys

PORT = 5000

GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.OUT)
GPIO.setwarnings(False)
pwm=GPIO.PWM(17,100)  
pwm.start(5)
def rotation(sens,angle, duree):
    ajoutAngle = 5
   
    if(sens==1):
    	duty1 = float(angle)/10 + ajoutAngle
        pwm.ChangeDutyCycle(duty1)
        time.sleep(duree)
    if(sens==2):
    	angle = angle+ 180
    	duty1 = float(angle)/10 + ajoutAngle
        pwm.ChangeDutyCycle(duty1)
        time.sleep(duree)
   

            
def serveurTcp():
    mySocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:                                                        
        mySocket.bind(('', PORT))
    except socket.error:         
        print ("La liaison du socket a l'adresse choisie a echoue.")
        sys.exit()                                                  
    while 1:      
        print("Serveur pret, en attente de requete")
        mySocket.listen(5)                          
        connexion, adresse = mySocket.accept()
        print ("Client connecte, adresse IP %s, port %s" % (adresse[0],adresse[1]))
        connexion.send("Vous etes connete au serveur de Joseline. Envoyez vos messages.\n")                                 
        while 1:                        
	    msgClient = connexion.recv(1024)
            print( "C>", msgClient)
            if( msgClient.upper() == "FIN" or msgClient ==""):
                break     
	    msgClien=msgClient.split(',')
	    print("L>",msgClien)
	    sens = int( msgClien[0]) 
	    angle= int( msgClien[1]) 
	    duree= int( msgClien[2])
	    print(sens,angle,duree) 
	    rotation(sens,angle,duree)                             
                                                                   
        connexion.send("Au revoir !")                                    
        print ("Connexion interrompue.")                                 
        connexion.close()                                                
                                         
if __name__=='__main__':                                      
                                                              
    serveurTcp()   
