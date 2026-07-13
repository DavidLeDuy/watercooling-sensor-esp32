// prevent the console window from opening when running the exe remove when debugging
#![windows_subsystem = "windows"]

use std::io::{BufRead, BufReader};
use std::time::Duration;
use std::thread;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let mut file_path = None;
    let mut com_port = None;

    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "-f" | "--file" => {
                if i + 1 < args.len() {
                    file_path = Some(args[i + 1].clone());
                    i += 2;
                } else {
                    eprintln!("Error: Missing value for --file");
                    std::process::exit(1);
                }
            }
            "-c" | "--com" => {
                if i + 1 < args.len() {
                    com_port = Some(args[i + 1].clone());
                    i += 2;
                } else {
                    eprintln!("Error: Missing value for --com");
                    std::process::exit(1);
                }
            }
            _ => {
                eprintln!("Unknown argument: {}", args[i]);
                std::process::exit(1);
            }
        }
    }

    let file_path = match file_path {
        Some(f) => f,
        None => {
            eprintln!("Error: --file is required");
            eprintln!("Usage: rust-watcher -f <path> -c <com_port>");
            std::process::exit(1);
        }
    };

    let com_port = match com_port {
        Some(c) => c,
        None => {
            eprintln!("Error: --com is required");
            eprintln!("Usage: rust-watcher -f <path> -c <com_port>");
            std::process::exit(1);
        }
    };

    println!("Starting Watercooling Sensor Watcher...");
    println!("Target File: {}", file_path);
    println!("COM Port: {}", com_port);

    loop {
        // Check if the port is in the list of available ports
        match serialport::available_ports() {
            Ok(ports) => {
                let found = ports.iter().any(|p| p.port_name.eq_ignore_ascii_case(&com_port));
                if !found {
                    println!("Port {} not available. Retrying in 2 seconds...", com_port);
                    thread::sleep(Duration::from_secs(2));
                    continue;
                }
            }
            Err(_) => {
                // If listing fails, try opening anyway
            }
        }

        // Attempt to open the serial port
        let port = match serialport::new(&com_port, 115_200)
            .timeout(Duration::from_secs(5))
            .open()
        {
            Ok(p) => p,
            Err(e) => {
                println!(
                    "Failed to open port {}: {}. Retrying in 2 seconds...",
                    com_port, e
                );
                thread::sleep(Duration::from_secs(2));
                continue;
            }
        };

        println!("Connected to {} successfully.", com_port);
        println!("Writing temperature to {}...", file_path);

        let mut reader = BufReader::new(port);
        let mut line = String::new();

        loop {
            line.clear();
            match reader.read_line(&mut line) {
                Ok(0) => {
                    // EOF reached (connection closed)
                    println!("EOF reached on serial port.");
                    break;
                }
                Ok(_) => {
                    let trimmed = line.trim();
                    if trimmed.is_empty() {
                        continue;
                    }

                    // Attempt to parse the temperature
                    if let Ok(temp) = trimmed.parse::<f64>() {
                        println!("Temperature: {}", temp);
                        let rounded = temp.round() as i32;
                        let temp_str = rounded.to_string();
                        // Write to file
                        if let Err(e) = std::fs::write(&file_path, temp_str.as_bytes()) {
                            eprintln!("Failed to write to file {}: {}", file_path, e);
                        }
                    } else {
                        // Log device errors/messages (e.g. "Error: Sensor Disconnected")
                        println!("Device log: {}", trimmed);
                    }
                }
                Err(ref e) if e.kind() == std::io::ErrorKind::TimedOut => {
                    // Timeout is expected if sensor doesn't send data within 5s, continue reading
                    continue;
                }
                Err(e) => {
                    println!("Read error: {}. Reconnecting...", e);
                    break;
                }
            }
        }

        println!("Disconnected. Reconnecting in 2 seconds...");
        thread::sleep(Duration::from_secs(2));
    }
}
