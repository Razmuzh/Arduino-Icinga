#!/usr/bin/python
#-*- coding: utf-8 -*-

import paho.mqtt.publish as publish
import json, requests, warnings, time
from requests.auth import HTTPBasicAuth
warnings.filterwarnings("ignore")




class Icinga:
  http_protocol = "http"
  icingaweb2_uri = "/icingaweb2"
  icinga_host = "XXX"
  icinga_user = "XXX"
  icinga_pw = "XXX"
  if icinga_user == "" or icinga_pw == "":
      raise SystemExit("Require api_user or api_secret")

  def getStuff(self, status, object):
    stuff=[]
    status=str(status)

    headers = { 'Accept': 'application/json', 'X-HTTP-Method-Override': 'GET' }
    if object == "host" or object == "service":
      if status == "1":
        url = self.http_protocol+"://"+self.icinga_host+self.icingaweb2_uri+"/monitoring/list/"+object+"s?"+object+"_state="+status+"&sort="+object+"_severity&"+object+"_unhandled=1&format=json"
      elif status == "2":
        url = self.http_protocol+"://"+self.icinga_host+self.icingaweb2_uri+"/monitoring/list/"+object+"s?"+object+"_state="+status+"&"+object+"_handled=0&format=json"
      else:
        url = self.http_protocol+"://"+self.icinga_host+self.icingaweb2_uri+"/monitoring/list/"+object+"s?"+object+"_problem="+ str(status) +"&format=json"

    try:
      r = requests.get(url, auth=HTTPBasicAuth(self.icinga_user, self.icinga_pw), verify=False, timeout=15, allow_redirects=False, headers=headers)
    except:
      print("fail|color=red")
      print("---")
      raise SystemExit("Icinga2 down or wrong credentials")
    if(r.status_code == 200):
      jresults = json.loads(r.content)
      for i in jresults:
        if object == "hostgroup":
          stuff.append(i[object+"_name"])
        else:
          stuff.append(i[object+"_display_name"])
    else:
      print("Fail|color=red")
      raise SystemExit("wrong status code"+str(r.status_code))
    return(stuff)
i = Icinga()


while True:
	# Output ###########
	down = i.getStuff(1,"host")
	up = i.getStuff(0,"host")
	servcrit = i.getStuff(2,"service")
	servok = i.getStuff(0,"service")
	servwarn = i.getStuff(1,"service")
	servunkn = i.getStuff(3,"service")

	publish.single("server/up", len(up), hostname="localhost")
	publish.single("server/down", len(down), hostname="localhost")
	publish.single("service/ok", len(servok), hostname="localhost")
	publish.single("service/warning", len(servwarn), hostname="localhost")
	publish.single("service/critical", len(servcrit), hostname="localhost")
	publish.single("service/unknown", len(servunkn), hostname="localhost")
	time.sleep(15)

