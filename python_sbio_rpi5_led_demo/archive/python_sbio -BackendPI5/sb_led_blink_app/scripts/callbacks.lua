local color = nil

--- @param gre#context mapargs
function blinkBlue(mapargs)
  color = "blue"
  sendData()
end


--- @param gre#context mapargs
function blinkWhite(mapargs)
  color = "white"
  sendData()
end


--- @param gre#context mapargs
function blinkRed(mapargs)
  color = "red"
  sendData()
end

--- @param gre#context mapargs
function sendData(mapargs)
  local event_name = "blink"
  local format_string = "1s0 messege"
  local data = { messege = color }
  local channel = "sbio_backend"
  gre.send_event_data(event_name,format_string,data,channel)
end


--- @param gre#context mapargs
function ackFromBackend(mapargs)
  local event = mapargs.context_event_data
  local ackReply  = event.ack
  local data_table  = {}
  local lastmsg_table  = {}
  local selectedLed = nil
  local stateLed = nil
  
  lastmsg_table["Layer.testMsgBackend.text"] = ackReply
  gre.set_data( lastmsg_table )
 
  selectedLed, stateLed = string.match(ackReply, "(.*)%-(.*)")
  print(selectedLed)
  print(stateLed)
     
  if selectedLed == ("blue") then
    selectedLed = "Layer.BlueOnOff.text"    
  elseif selectedLed == ("white") then
    selectedLed = "Layer.WhiteOnOff.text"
  elseif selectedLed == ("red") then
    selectedLed = "Layer.RedOnOff.text"
  end
  --print(ackReply)
  data_table[selectedLed] = stateLed
  gre.set_data( data_table )

end
