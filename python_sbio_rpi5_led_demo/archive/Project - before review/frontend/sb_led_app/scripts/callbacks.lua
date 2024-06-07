--GLOBAL VARIABLES
local color = nil

--BLUE
--- @param gre#context mapargs
function blinkBlue(mapargs)
  color = "blue"
  sendData()
end

--WHITE
--- @param gre#context mapargs
function blinkWhite(mapargs)
  color = "white"
  sendData()
end

--RED
--- @param gre#context mapargs
function blinkRed(mapargs)
  color = "red"
  sendData()
end

--SEND TO PYTHON BACKEND
--- @param gre#context mapargs
function sendData(mapargs)
  local event_name = "blink"
  local format_string = "1s0 messege"
  local data = { messege = color }
  local channel = "sbio_backend"
  gre.send_event_data(event_name,format_string,data,channel)
end


--RECEIVE FROM PYTHON BACKEND
--- @param gre#context mapargs
function ackFromBackend(mapargs)
  local event = mapargs.context_event_data
  local ackReply  = event.ack
  local data_table  = {}
  local lastmsg_table  = {}
  local selectedLed = nil   --COLOR
  local stateLed = nil      --ON-OFF
  
  --last messege received 
  lastmsg_table["Layer.testMsgBackend.text"] = ackReply
  gre.set_data( lastmsg_table )
 
  --led state update
  selectedLed, stateLed = string.match(ackReply, "(.*)%-(.*)")
  selectedLed = "Layer."..selectedLed.."OnOff.text"
  data_table[selectedLed] = stateLed
  gre.set_data( data_table )
  
  --print(selectedLed)
  --print(stateLed)
  
end
