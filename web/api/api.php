<?php
	/*Tham khao:
		http://coreymaynard.com/blog/creating-a-restful-api-with-php/
		http://www.sitepoint.com/best-practices-rest-api-scratch-introduction/
	*/
abstract class API {
	protected $method = '';
	protected $endpoint = '';
	protected $verb = '';
	
	protected $args = Array();
	
	protected $file = null;
	
	public function __construct($request) {
		header("Access-Control-Allow-Orgin: *");
		header("Access-Control-Allow-Methods: *");
		header("Content-Type: application/json");
		
		$this->args = explode('/', rtrim($request, '/'));
		$this->endpoint = array_shift($this->args);
		
		if(array_key_exists(0, $this->args) && !is_numeric($this->args[0])) {
			$this->verb = array_shift($this->args);
		}
	}
	
	public function process_api() {
		
	}
}
?>