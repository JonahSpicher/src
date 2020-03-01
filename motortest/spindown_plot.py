import numpy as np
import matplotlib.pyplot as plt

#First solve angles from calibration data
# angles = [1846, 6037, 10140, 14335]
# degrees = [0, np.pi*0.5, np.pi, 3*np.pi/2]
# a2d = np.polyfit(angles, degrees, 1)
# plt.scatter(angles, degrees)
# plt.plot(angles, a2d[0]*np.array(angles) + a2d[1])
# plt.title("Calibration Data")
# plt.xlabel("Angle Recorded by Encoder")
# plt.ylabel("Angle in Degrees")
# plt.figure()
#
# #Solve and plot 100% duty cycle
# data = np.unwrap(a2d[0]*np.load("spindown100.npy") + a2d[1]) #Converts angle data to degrees
# ts = np.array(range(len(data)))*8/len(data) #Converts indices to seconds
#
# plt.plot(ts[240:548], data[240:548]) #Only plots specified range where spinning happened
# plt.title("100% Duty Cycle")
# plt.xlabel("Time (s)")
# plt.ylabel("Angle (Deg)")
# plt.figure()
#
# #Solve and plot 50% duty cycle
# data = np.unwrap(a2d[0]*np.load("spindown50.npy") + a2d[1]) #Converts angle data to degrees
# ts = np.array(range(len(data)))*8/len(data) #Converts indices to seconds
#
# plt.plot(ts[290:600], data[290:600]) #Only plots specified range where spinning happened
# plt.title("50% Duty Cycle")
# plt.xlabel("Time (s)")
# plt.ylabel("Angle (Deg)")
# plt.show()


angle = np.load("texture-angle.npy")
dc = np.load("texture-dc.npy")
#measured_vs = np.load("damp-velocity.npy")
#vs = np.gradient(angle)
#print(len(vs))
plt.plot(angle[130:-50], dc[130:-50])

plt.title('Textured Duty Cycle Response')
plt.xlabel('Angular Displacement')
plt.ylabel('Duty Cycle')
plt.show()
