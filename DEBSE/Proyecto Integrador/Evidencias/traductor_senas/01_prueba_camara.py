import cv2

cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("❌ ERROR: No se pudo abrir la cámara")
    exit()

print("✅ Cámara abierta. Presiona ESC para salir")

while True:
    ret, frame = cap.read()
    if not ret:
        break
    cv2.imshow('Prueba de Cámara', frame)
    if cv2.waitKey(1) == 27:  # ESC
        break

cap.release()
cv2.destroyAllWindows()