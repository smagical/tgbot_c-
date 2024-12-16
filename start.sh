#!/bin/bash

SHELL_FOLDER=$(dirname $(readlink -f "$0"))
APP_EXE="${SHELL_FOLDER}/tg_bot"
APP_CONFIG="${SHELL_FOLDER}/bot_config.ini"
APP_RUN_CMD="${APP_EXE}  ${APP_CONFIG}"
#echo $APP_RUN_CMD
#检查程序是否在运行
is_exist(){
  pid=$(ps -ax|grep ${APP_EXE}|grep -v grep|awk '{print $2}' )
  echo "##################"
  echo $pid
  echo "##################"
  #如果不存在返回1，存在返回0
  if [ -z "${pid}" ]; then
   return 1
  else
    return 0
  fi
}

stop(){
  is_exist
  if [ $? -eq 0 ]; then
      kill -9 ${pid}
  else
    echo "$APP not run"
  fi
}

start(){
  is_exist
  if [ $? -eq 0 ]; then
      echo "${APP_EXE} is run,pid is ${pid}"
  else
      nohup ${APP_RUN_CMD} >> ${APP_EXE}.log &
  fi
}

restart(){
  stop
  start
}

#根据输入参数，选择执行对应方法，不输入则执行使用说明
case "$1" in
  "start")
    start
    ;;
  "stop")
    stop
    ;;
  "status")
    status
    ;;
  "restart")
    restart
    ;;
  *)
    start
    ;;
esac

