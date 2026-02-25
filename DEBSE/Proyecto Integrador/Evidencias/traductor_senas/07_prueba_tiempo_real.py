import cv2
import mediapipe as mp
import numpy as np
import pickle
import tensorflow as tf
import time
from pathlib import Path

class ReconocedorMultimodal:
    def __init__(self):
        self.modelos_dir = Path("modelos")
        self.modelo = tf.keras.models.load_model(self.modelos_dir / "modelo_lstm.h5")
        with open(self.modelos_dir / "clases_lstm.pkl", "rb") as f:
            self.clases = pickle.load(f)
        
        self.mp_hands = mp.solutions.hands
        self.hands = self.mp_hands.Hands(static_image_mode=False, max_num_hands=2,
                                          min_detection_confidence=0.7)
        self.mp_draw = mp.solutions.drawing_utils
        
        self.secuencia = []  # acumula frames para la seña actual
        self.max_len = self.modelo.input_shape[1]  # longitud máxima que espera el modelo
        self.modo = "deletreo"  # o "palabras"
        self.texto = ""
        self.ultimo_tiempo = time.time()
        
    def extraer_landmarks(self, results):
        landmarks = []
        if results.multi_hand_landmarks:
            manos = results.multi_hand_landmarks
            if len(manos) >= 1:
                for lm in manos[0].landmark:
                    landmarks.extend([lm.x, lm.y, lm.z])
            else:
                landmarks.extend([0.0]*63)
            if len(manos) >= 2:
                for lm in manos[1].landmark:
                    landmarks.extend([lm.x, lm.y, lm.z])
            else:
                landmarks.extend([0.0]*63)
        else:
            landmarks = [0.0]*126
        return np.array(landmarks)
    
    def predecir_secuencia(self, secuencia):
        # Rellenar o truncar a la longitud esperada
        if len(secuencia) > self.max_len:
            secuencia = secuencia[-self.max_len:]
        elif len(secuencia) < self.max_len:
            pad = np.zeros((self.max_len - len(secuencia), 126))
            secuencia = np.vstack([pad, secuencia])
        secuencia = secuencia.reshape(1, self.max_len, 126)
        pred = self.modelo.predict(secuencia, verbose=0)[0]
        clase_idx = np.argmax(pred)
        confianza = pred[clase_idx]
        return self.clases[clase_idx], confianza
    
    def run(self):
        cap = cv2.VideoCapture(0)
        print("🎥 Reconocimiento multimodal - Presiona:")
        print("  'd' - modo deletreo (letra por letra)")
        print("  'p' - modo palabras (señas completas)")
        print("  ESPACIO - capturar (en modo deletreo captura letra; en modo palabras inicia/finaliza secuencia)")
        print("  ESC - salir")
        
        capturando = False
        while True:
            ret, frame = cap.read()
            if not ret:
                break
            frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            results = self.hands.process(frame_rgb)
            
            # Dibujar landmarks
            if results.multi_hand_landmarks:
                for hand_landmarks in results.multi_hand_landmarks:
                    self.mp_draw.draw_landmarks(frame, hand_landmarks, self.mp_hands.HAND_CONNECTIONS)
            
            # Mostrar modo y texto
            cv2.putText(frame, f"Modo: {self.modo}", (10,30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,255,0),2)
            cv2.putText(frame, f"Texto: {self.texto}", (10, frame.shape[0]-20), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,255,255),2)
            
            if self.modo == "palabras" and capturando:
                cv2.putText(frame, "GRABANDO...", (frame.shape[1]-200,30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,255),2)
            
            cv2.imshow("Reconocedor Multimodal", frame)
            key = cv2.waitKey(1) & 0xFF
            
            if key == ord('d'):
                self.modo = "deletreo"
                self.secuencia = []
                capturando = False
                print("Modo deletreo activado")
            elif key == ord('p'):
                self.modo = "palabras"
                self.secuencia = []
                capturando = False
                print("Modo palabras activado")
            elif key == ord(' '):
                if self.modo == "deletreo":
                    # Capturar un frame y predecir letra
                    landmarks = self.extraer_landmarks(results)
                    if np.any(landmarks):  # si hay mano
                        # Crear una secuencia de un solo frame
                        secuencia = np.array([landmarks])
                        letra, conf = self.predecir_secuencia(secuencia)
                        if conf > 0.7:
                            self.texto += letra
                            print(f"Letra: {letra} (confianza: {conf:.2f})")
                else:  # modo palabras
                    if not capturando:
                        # empezar a grabar
                        capturando = True
                        self.secuencia = []
                        print("Inicio de grabación de palabra")
                    else:
                        # terminar y predecir
                        capturando = False
                        if len(self.secuencia) > 0:
                            palabra, conf = self.predecir_secuencia(np.array(self.secuencia))
                            if conf > 0.6:
                                self.texto += " " + palabra + " "
                                print(f"Palabra: {palabra} (confianza: {conf:.2f})")
                            else:
                                print("No se reconoció la seña")
            
            # En modo palabras, si estamos grabando, acumular frames
            if self.modo == "palabras" and capturando:
                landmarks = self.extraer_landmarks(results)
                self.secuencia.append(landmarks)
            
            if key == 27:  # ESC
                break
        
        cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    recognizer = ReconocedorMultimodal()
    recognizer.run()