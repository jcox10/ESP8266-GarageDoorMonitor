-- LUA script for Domoticz
-- Sends notification if door has been open for longer than 10 minutes, then sends notification once it is closed
-- Place script in domoticz/var/scripts/lua

debug = false
commandArray = {}

if (debug) then
	print("State of garage door: "..otherdevices['Garage Door'])
end

if (otherdevices['Garage Door'] == 'Closed') then
	if(uservariables["GarageDoorNotificationSent"] == "true") then
		commandArray['Variable:GarageDoorNotificationSent']="false"
		commandArray['SendNotification']='DOMOTICZ-Garage door alert#Garage door is now closed.'
	end
	return commandArray
end

t1 = os.time()
s = otherdevices_lastupdate['Garage Door']
-- returns a date time like 2013-07-11 17:23:12

year = string.sub(s, 1, 4)
month = string.sub(s, 6, 7)
day = string.sub(s, 9, 10)
hour = string.sub(s, 12, 13)
minutes = string.sub(s, 15, 16)
seconds = string.sub(s, 18, 19)

t2 = os.time{year=year, month=month, day=day, hour=hour, min=minutes, sec=seconds}
difference = (os.difftime (t1, t2))

if (debug) then
	print("Time difference since last update: "..difference)
end

if (difference > 600 and uservariables["GarageDoorNotificationSent"] == "false") then
	commandArray['SendNotification']='DOMOTICZ-Garage door alert#The garage door has been open for more than 10 minutes!'
	commandArray['Variable:GarageDoorNotificationSent']="true"
end 

return commandArray
