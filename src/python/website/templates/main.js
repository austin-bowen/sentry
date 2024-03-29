class MotorController {
  constructor(socket) {
    this.socket = socket

    this.t_last_drive_command = new Date()
  }

  drive(linear, angular, force=false) {
    const dt_last_drive_command = (new Date() - this.t_last_drive_command) / 1000

    if (force || dt_last_drive_command > 0.1) {
      console.log('drive', linear, angular)
      this.socket.emit('motorController.drive', linear, angular)
      this.t_last_drive_command = new Date()
    }
  }

  stop() {
    console.log('stop')
    this.socket.emit('motorController.stop')
  }
}

const videoCanvas = document.getElementById('video_canvas')
const driveKeys = new Set(['W', 'A', 'S', 'D'])
const pressedKeys = new Set()
var motorController = null

function main() {
  const socket = io()

  setupButtons(socket)
  setupVideoFeed()
  setupMotorController(socket)
  setupKeyboard()
  setupJoystick()
  setupStatusDisplay(socket)
}

function setupButtons(socket) {
  // Shutdown button
  var button = document.getElementById('shutdown_button')
  button.onclick = () => {
    const shutdown = confirm('Shutdown Sentry?')

    if (shutdown) {
      console.log('Shutdown')
      socket.emit('shutdown')
    }
  }

  // Reboot button
  button = document.getElementById('reboot_button')
  button.onclick = () => {
    const reboot = confirm('Reboot Sentry?')

    if (reboot) {
      console.log('Reboot')
      socket.emit('reboot')
    }
  }

  // Restart service button
  button = document.getElementById('restart_service_button')
  button.onclick = () => {
    const reboot = confirm('Restart Sentry service?')

    if (reboot) {
      console.log('Restart service')
      socket.emit('restart_service')
    }
  }
}

function setupVideoFeed() {
  const videoFeed = document.getElementById('video_feed')
  const videoFeedUrl = new URL(window.location.href)
  videoFeedUrl.port = {{ video_stream_port }}
  videoFeedUrl.pathname = '/'
  videoFeed.src = videoFeedUrl.href

  setupVideoCanvas()
}

function setupVideoCanvas() {
  const ctx = videoCanvas.getContext('2d')
  ctx.globalAlpha = 0.5

  // Crosshair
  const centerX = 800 / 2
  const centerY = 600 / 2
  const size = 5
  ctx.moveTo(centerX - size, centerY)
  ctx.lineTo(centerX + size, centerY)
  ctx.moveTo(centerX, centerY - size)
  ctx.lineTo(centerX, centerY + size)

  // Left drive guide
  ctx.moveTo(145, 600)
  ctx.lineTo(270, 450)
  ctx.stroke()

  // Right drive guide
  ctx.moveTo(800 - 145, 600)
  ctx.lineTo(800 - 270, 450)
  ctx.stroke()
}

function setupMotorController(socket) {
  motorController = new MotorController(socket)

  window.onbeforeunload = () => {
    motorController.stop()
  }
}

function setupKeyboard() {
  document.addEventListener('keydown', event => {
    const key = event.key.toUpperCase()
    if (driveKeys.has(key) && !pressedKeys.has(key)) {
      pressedKeys.add(key)
      driveWithKeys()
    }
  })

  document.addEventListener('keyup', event => {
    const key = event.key.toUpperCase()
    if (driveKeys.has(key) && pressedKeys.has(key)) {
      pressedKeys.delete(key)
      driveWithKeys()
    }
  })
}

function driveWithKeys() {
  const forward = 'W'
  const backward = 'S'
  const left = 'A'
  const right = 'D'

  var linear = 0
  if (pressedKeys.has(forward) && !pressedKeys.has(backward)) {
    linear = getMaxLinearSpeed()
  } else if (!pressedKeys.has(forward) && pressedKeys.has(backward)) {
    linear = -getMaxLinearSpeed()
  }

  var angular = 0
  if (pressedKeys.has(left) && !pressedKeys.has(right)) {
    angular = getMaxAngularSpeed()
  } else if (!pressedKeys.has(left) && pressedKeys.has(right)) {
    angular = -getMaxAngularSpeed()
  }

  if (Math.abs(linear) > 0 && Math.abs(angular) > 0) {
    linear *= 0.707
    angular *= 0.707
  }

  if (linear != 0 || angular != 0) {
    motorController.drive(linear, angular, force=true)
  } else {
    motorController.stop()
  }
}

function setupJoystick() {
  const joystickManager = nipplejs.create({
    zone: document.getElementById('joystick'),
    color: 'black',
    position: {left: '50%', bottom: '8rem'},
    mode: 'static',
  })

  joystickManager.on('end', function(event, joystick) {
    motorController.stop()
  })

  joystickManager.on('move', function(event, joystick) {
    handleJoystickMovement(joystick.vector.x, joystick.vector.y)
  })
}

function handleJoystickMovement(x, y) {
  const linear = joystickToLinearVelocity(y)
  const angular = joystickToAngularVelocity(x)

  motorController.drive(linear, angular)
}

function joystickToLinearVelocity(position) {
  if (Math.abs(position) < 0.1) {
    return 0.0
  }

  return position * getMaxLinearSpeed()
}

function getMaxLinearSpeed() {
  return 0.25
}

function joystickToAngularVelocity(position) {
  if (Math.abs(position) < 0.1) {
    return 0.0
  }

  return -position * getMaxAngularSpeed()
}

function getMaxAngularSpeed() {
  return Math.PI / 3
}

function setupStatusDisplay(socket) {
  const statusDiv = document.getElementById('status')

  socket.on('status', status => {
    statusDiv.innerHTML = status
  })
}

main()
