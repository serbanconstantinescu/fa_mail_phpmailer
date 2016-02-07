<?php

include_once($path_to_root . "/modules/mail/includes/PHPMailer/class.smtp.php");
include_once($path_to_root . "/modules/mail/includes/PHPMailer/class.phpmailer.php");


class email {
    public $phpmailer = null;
    public $charset = 'ISO-8859-1';

    public function __construct($name, $email) {
	//create phpmailer object
	$this->phpmailer = new PHPMailer;

	//set from
	$this->phpmailer->setFrom($email, $name);

	//setup bcc if one is set
	$bcc = get_company_pref('bcc_email');
	if ($bcc)
	    $this->phpmailer->addBCC($bcc);


	//Tell PHPMailer what to use: php's mail or smtp
	$_mail_type = get_company_pref('mail_type');
	switch($_mail_type) {
	    case 'SMTP':
		$this->phpmailer->isSMTP();
		break;
	    case 'MAIL':
	    default:
		$this->phpmailer->isMail();
		break;
	}
	//if we are sending via SMTP, make specific settings
	if ($_mail_type == 'SMTP') {
	    //Set the hostname of the mail server
	    $this->phpmailer->Host = get_company_pref('smtp_host');

	    //Set the SMTP port number - 587 for authenticated TLS, a.k.a. RFC4409 SMTP submission
	    $this->phpmailer->Port = get_company_pref('smtp_port');


	    //Whether to use SMTP authentication
	    $_smtp_username = get_company_pref('smtp_username');
	    $_smtp_password = get_company_pref('smtp_password');
	    if (!empty($_smtp_password) && !empty($_smtp_username)) {
		$this->phpmailer->SMTPAuth = true;
		$this->phpmailer->Username = $_smtp_username;
		$this->phpmailer->Password = $_smtp_password;
	    }

	    //Set the encryption system to use - ssl (deprecated) or tls
	    $_smtp_secure = get_company_pref('smtp_secure');
	    switch($_smtp_secure) {
		case 'ssl':
		case 'tls':
		    $this->phpmailer->SMTPSecure = $_smtp_secure;
		default:
		case 'none':
		    //just do nothing
		    //$this->phpmailer->SMTPSecure = $_smtp_secure;
		    break;
	    }

	    //Enable SMTP debugging
	    // 0 = off (for production use)
	    // 1 = client messages
	    // 2 = client and server messages
	    $this->phpmailer->SMTPDebug = 0;

	    //Ask for HTML-friendly debug output
	    $this->phpmailer->Debugoutput = function($str, $level) {
		error_log("DEBUG: class.mail.inc: PHPMailerdebug: $level: $str");

	    };
	}
	








    }

    private function mailparse_rfc822($rfc822_email_string) {
	$data = array();
	// match all words and whitespace, will be terminated by '<'
        $name = preg_match('/[\w\s]+/', $rfc822_email_string, $matches);
        $data['name'] = trim($matches[0]);
                
        // extract parts between the two parentheses
        $address = preg_match('/(?:<)(.+)(?:>)$/', $rfc822_email_string, $matches);
    	$data['email'] = $matches[1];
        
        return $data;
    }
    
    public function to($mail) {
	error_log("DEBUG: sending mail to: $mail");
	$data = $this->mailparse_rfc822($mail);

	error_log("DEBUG: parsed mail:" . print_r($data, true));
	
	$this->phpmailer->addAddress($data['email'], $data['name']);
    }
    
    public function cc($mail) {
	$data = $this->mailparse_rfc822($mail);
	$this->phpmailer->addCC($data['email'], $data['name']);
    }
    
    public function bcc($mail) {
	$data = $this->mailparse_rfc822($mail);
	$this->phpmailer->addBCC($data['email'], $data['name']);
    }
    
    public function subject($subject) {
	$this->phpmailer->Subject = $subject;
    }
    
    public function attachment($file) {
	$this->phpmailer->addAttachment($file);
    }
    
    public function text($text) {
	$this->phpmailer->Body = $text;
    }
    
    public function html($html) {
	$this->phpmailer->msgHTML($html);
    }
    
    public function send() {
	$this->phpmailerCharSet = $this->charset;

	//send the message, check for errors
	if (!$this->phpmailer->send()) {
	    error_log('Mailer error:' . $this->phpmailer->ErrorInfo);
	    //echo "Mailer Error: " . $this->phpmailer->ErrorInfo;
	    return 0;
	} else {
	    return 1;
	}
    }
}




class old_email_class
{
    var $to = array();
    var $cc = array();
    var $bcc = array();
    var $attachment = array();
    var $boundary = "";
    var $header = "";
    var $subject = "";
    var $body = "";
	var $charset = 'ISO-8859-1';
	var $add_params;
	
    function email($name, $mail)
    {
        $this->boundary = md5(uniqid(time()));
		$this->header = "From: $name <$mail>\n";
		$bcc = get_company_pref('bcc_email');
		if ($bcc)
			$this->bcc[] = $bcc;
    }

    function to($mail)
    {
    	$this->to[] = $mail;
    }

    function cc($mail)
    {
    	$this->cc[] = $mail;
    }

    function bcc($mail)
    {
    	$this->bcc[] = $mail;
    }

    function attachment($file)
    {
		$this->attachment[] = $file;
    }

    function subject($subject)
    {
    	$this->subject = $subject;
    }

    function text($text)
    {
        $this->body = "--$this->boundary\n";
	    $this->body .= "Content-Type: text/plain; charset=\"{$this->charset}\"\n";
	    $this->body .= "Content-Transfer-Encoding: 8bit\n\n";
	    $this->body .= $text."\n";
    }

    function html($html)
    {
        $this->body = "--$this->boundary\n";
	    $this->body .= "Content-Type: text/html; charset=\"{$this->charset}\"\n";
	    $this->body .= "Content-Transfer-Encoding: quoted-printable\n\n";
	    $this->body .= "<html><body>\n".$html."\n</body></html>\n";
    }

	function mime_type($filename)
	{
		$file = basename($filename, '.zip');
		if ($filename == $file . '.zip') return 'application/x-zip-compressed';
		$file = basename($filename, '.pdf');
		if ($filename == $file . '.pdf') return 'application/pdf';
		$file = basename($filename, '.csv');
		if ($filename == $file . '.csv') return 'application/vnd.ms-excel';
		$file = basename($filename, '.tar');
		if ($filename == $file . '.tar') return 'application/x-tar';
		$file = basename($filename, '.tar.gz');
		if ($filename == $file . '.tar.gz') return 'application/x-tar-gz';
		$file = basename($filename, '.tgz');
		if ($filename == $file . '.tgz') return 'application/x-tar-gz';
		$file = basename($filename, '.gz');
		if ($filename == $file . '.gz') return 'application/x-gzip';
		$file = basename($filename, '.html');
		if ($filename == $file . '.html') return 'text/html';
		return 'application/unknown';
	}

	function send()
    {
        // CC Empfänger hinzufügen
        $max = count($this->cc);
        if ($max > 0)
        {
            $this->header .= "Cc: ".$this->cc[0];
            for ($i = 1; $i < $max; $i++)
            {
                $this->header .= ", ".$this->cc[$i];
            }
            $this->header .= "\n";
        }
        // BCC Empfänger hinzufügen
        $max = count($this->bcc);
        if ($max > 0)
        {
            $this->header .= "Bcc: ".$this->bcc[0];
            for ($i = 1; $i < $max; $i++)
            {
                $this->header .= ", ".$this->bcc[$i];
            }
            $this->header .= "\n";
        }
		$this->header .= "Content-Type: multipart/mixed;\n boundary=\"$this->boundary\"\n";

        // Attachment hinzufügen
        $max = count($this->attachment);
        if ($max > 0)
        {
            for ($i = 0; $i < $max; $i++)
            {
                $file = fread(fopen($this->attachment[$i], "r"), filesize($this->attachment[$i]));
				$this->body .= "--".$this->boundary."\n";
				$this->body .= "Content-Type: " .$this->mime_type(basename($this->attachment[$i])). "; name=\"".basename($this->attachment[$i])."\"\n";
				$this->body .= "Content-Transfer-Encoding: base64\n";
				$this->body .= "Content-Disposition: attachment; filename=\"".basename($this->attachment[$i])."\"\n\n";
				$this->body .= chunk_split(base64_encode($file),"72","\n");
                $file = "";
            }
        }
			$this->body .= "--".$this->boundary."--\n";

		$ret = 0;
        foreach($this->to as $mail)
        {
			if (mail($mail, $this->subject, $this->body, $this->header, $this->add_params))
				$ret++;
        }
        return $ret;
    }
}
?>
