import cv2
import mediapipe as mp

mp_hands = mp.solutions.hands
hands = mp_hands.Hands(static_image_module=False,          # Movimiento de las manos... ff
                          max_num_hands=2,
                          min_detection_confidence=0.5,
                          min_tracking_confidence=0.5)
mp_draw = mp.solutions.drawing_utils

cap = cv2.VideoCapture(0)

print("🎯 Detector de Manos - Mueve tu mano frente a la cámara")
print("Presiona ESC para salir")

while cap.isOpened():
    success, frame = cap.read()
    if not success:
        break
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(frame_rgb)
    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            mp_draw.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
    cv2.imshow('Detector de Manos', frame)
    if cv2.waitKey(1) == 27:
        break

cap.release()
cv2.destroyAllWindows()