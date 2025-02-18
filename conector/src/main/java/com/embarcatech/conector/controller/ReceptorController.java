package com.embarcatech.conector.controller;

import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/")
public class ReceptorController {

    @PostMapping("enviar_amostras")
    public void receberDadosBitdoglab(@RequestBody(required = false)  String json){
        System.out.println(json);
    }
}
